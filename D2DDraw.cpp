#include "D2DDraw.h"

D2DDraw::D2DDraw() :
	pD2DFac(NULL),
	pRT(NULL),
	pBrush(NULL),
	m_pWicBitmap(NULL),
	m_pWicImagingFactory(NULL),
	m_pWicDecoder(NULL),
	m_pWicFrameDecoder(NULL),
	pIWriBrush(NULL),
	pIDWriteFactory(NULL),
	pWriBrush(NULL),
	p_Scaler(NULL),
	//g_pLayer(NULL),
	rBrush(NULL)
{
	//TFEffect = 0;

	memset(&rc, 0, sizeof(rc));
}


D2DDraw::~D2DDraw()
{
	SafeRelease(pRT);
	SafeRelease(pBrush);

	SafeRelease(pD2DFac);

	SafeRelease(pIWriBrush);
	SafeRelease(pIDWriteFactory);
	SafeRelease(pWriBrush);

	SafeRelease(p_Scaler);

	//SafeRelease(g_pLayer);

	SafeRelease(rBrush);
	ReleaseText();
}

void D2DDraw::LoadD2D(HWND hwnd) {

	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&pD2DFac);

	GetClientRect(hwnd, &rc);

	HRESULT Hr = pD2DFac->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU(
				rc.right - rc.left,
				rc.bottom - rc.top)
		),
		&pRT
	);

	if (SUCCEEDED(Hr)) {
		pRT->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Green),
			&pBrush
		);
	}

	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWicImagingFactory));

	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&pIDWriteFactory));

	//hr = pRT->CreateLayer(NULL, &g_pLayer);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Create layer failed!", "Error", 0);
		return;
	}

}

void D2DDraw::ReleaseBMP()
{
	SafeRelease(m_pWicBitmap);
	SafeRelease(m_pWicDecoder);
	SafeRelease(m_pWicFrameDecoder);
}

void D2DDraw::BeginDr() {

	pRT->BeginDraw();

	pRT->Clear();
}

void D2DDraw::EndDr() {

	/*if (TFEffect) {

		pRT->PopLayer();
		TFEffect = false;

	}*/

	pRT->EndDraw();
}

D2DBitMap* D2DDraw::LoadBMP(LPCWSTR FileName, UINT destinationWidth, UINT destinationHeight)
{
	D2DBitMap* m_pD2d1Bitmap = NULL;

	IWICStream *pStream = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICFormatConverter *pConverter = NULL;

	HRESULT hr = S_OK;

	hr = m_pWicImagingFactory->CreateDecoderFromFilename(
		FileName,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&m_pWicDecoder
	);

	if (SUCCEEDED(hr))
	{

		// Create the initial frame.
		hr = m_pWicDecoder->GetFrame(0, &pSource);
	}
	else {
		return NULL;
	}

	IWICBitmapSource * pWicSource=nullptr;

	if (SUCCEEDED(hr)) {
		hr= pSource->QueryInterface(IID_PPV_ARGS(&pWicSource));
	}
	else{
		
		return NULL;
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pWicImagingFactory->CreateFormatConverter(&pConverter);
	}

	if (destinationWidth != 0 || destinationHeight != 0)
	{
		UINT originalWidth, originalHeight;
		hr = pSource->GetSize(&originalWidth, &originalHeight);
		if (SUCCEEDED(hr))
		{
			if (destinationWidth == 0)
			{
				FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
				destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
			}
			else if (destinationHeight == 0)
			{
				FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
				destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
			}

			hr = m_pWicImagingFactory->CreateBitmapScaler(&p_Scaler);
			if (SUCCEEDED(hr))
			{
				hr = p_Scaler->Initialize(
					pWicSource,
					destinationWidth,
					destinationHeight,
					WICBitmapInterpolationModeCubic
				);
			}
			if (SUCCEEDED(hr))
			{
				hr = pConverter->Initialize(
					p_Scaler,
					GUID_WICPixelFormat32bppPBGRA,
					WICBitmapDitherTypeNone,
					NULL,
					0.f,
					WICBitmapPaletteTypeMedianCut
				);
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRT->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			&m_pD2d1Bitmap
		);
	}

	ReleaseBMP();

	return m_pD2d1Bitmap;
}

D2DBitMap* D2DDraw::LoadAlphaBMP(LPCWSTR FileName, UINT destinationWidth, UINT destinationHeight, RGBA_F TransColor)
{
	D2DBitMap* m_pD2d1Bitmap = NULL;

	IWICStream *pStream = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmap *pWicBitmap = NULL;

	HRESULT hr = S_OK;

	hr = m_pWicImagingFactory->CreateDecoderFromFilename(
		FileName,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&m_pWicDecoder
	);

	if (SUCCEEDED(hr))
	{

		// Create the initial frame.
		hr = m_pWicDecoder->GetFrame(0, &pSource);
	}


	if (SUCCEEDED(hr))
	{
		hr = m_pWicImagingFactory->CreateFormatConverter(&pConverter);
	}

	if (destinationWidth != 0 || destinationHeight != 0)
	{
		UINT originalWidth, originalHeight;
		hr = pSource->GetSize(&originalWidth, &originalHeight);

		if (SUCCEEDED(hr))
		{
			hr = m_pWicImagingFactory->CreateBitmapFromSourceRect(
				pSource, 0, 0, (UINT)originalWidth, (UINT)originalHeight, &pWicBitmap);
		}

		/*实现像素操作--------------------------

		IWICBitmapLock *pILock = NULL;
		WICRect rcLock = { 0, 0, (int)originalWidth, (int)originalHeight };
		hr = pWicBitmap->Lock(&rcLock, WICBitmapLockWrite, &pILock);

		if (SUCCEEDED(hr))
		{
		UINT cbBufferSize = 0;
		BYTE *pv = NULL;

		if (SUCCEEDED(hr))
		{
		// 获取锁定矩形中左上角像素的指针
		hr = pILock->GetDataPointer(&cbBufferSize, &pv);

		for (unsigned int i = 0; i < cbBufferSize; i += 4)
		{
		if (pv[i + 3] != 0)
		{

		/*if(pv[i] == (BYTE)TransColor.B)
		if(pv[i + 1] == (BYTE)TransColor.G)
		if (pv[i + 2] == (BYTE)TransColor.R) {
		pv[i + 3] = (BYTE)TransColor.A;
		//MessageBox(NULL, "Life!", "debug", MB_OK);
		}

		/*char* str = NULL;
		sprintf_s(str,5, "%d", (int)pv[i]);



		D2D1_COLOR_F TransColor2=D2D1::ColorF(1, 0, 0, 1);

		pv[i] *= TransColor2.b;
		pv[i + 1] *= TransColor2.g;
		pv[i + 2] *= TransColor2.r;
		pv[i + 3] *= TransColor2.a;

		/*pv[i]=(BYTE)1;
		pv[i + 1] = (BYTE)1;
		pv[i + 2] = (BYTE)1;
		pv[i + 3] = (BYTE)1;
		}
		}
		}
		SafeRelease(pILock);

		}

		//完成像素操作------------------*/

		if (SUCCEEDED(hr))
		{
			if (destinationWidth == 0)
			{
				FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
				destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
			}
			else if (destinationHeight == 0)
			{
				FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
				destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
			}

			hr = m_pWicImagingFactory->CreateBitmapScaler(&p_Scaler);
			if (SUCCEEDED(hr))
			{
				hr = p_Scaler->Initialize(
					pWicBitmap,
					destinationHeight,
					destinationWidth,
					WICBitmapInterpolationModeCubic
				);
			}
			if (SUCCEEDED(hr))
			{
				hr = pConverter->Initialize(
					p_Scaler,
					GUID_WICPixelFormat32bppPBGRA,
					WICBitmapDitherTypeNone,
					NULL,
					0.f,
					WICBitmapPaletteTypeMedianCut
				);
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRT->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			&m_pD2d1Bitmap
		);
	}


	ReleaseBMP();

	return m_pD2d1Bitmap;

}

void D2DDraw::DrawBMP(D2DBitMap* BMP, int X, int Y, float Alpha)
{

	D2D1_SIZE_F rtSize = pRT->GetSize();
	if (BMP != NULL)
	{
		D2D1_SIZE_U sizeU = BMP->GetPixelSize();

		D2D1_RECT_F rectangle3 = D2D1::RectF(
			(float)X,
			(float)Y,
			(float)X + sizeU.width,
			(float)Y + sizeU.height
		);

		pRT->DrawBitmap(BMP, &rectangle3, Alpha);
	}

}

void D2DDraw::DrawBMP(D2DBitMap* BMP, int X, int Y, int Width, int Height, float Alpha,bool enlarge)
{
	if (enlarge == 1) {
		D2D1_SIZE_F rtSize = pRT->GetSize();
		if (BMP != NULL)
		{
			D2D1_SIZE_U sizeU = BMP->GetPixelSize();

			D2D1_RECT_F rectangle3 = D2D1::RectF(
				(float)X,
				(float)Y,
				(float)X + sizeU.width,
				(float)Y + sizeU.height
			);

			D2D1_RECT_F Zoom = D2D1::RectF(
				(float)X,
				(float)Y,
				(float)X + Width,
				(float)Y + Height
			);

			pRT->DrawBitmap(
				BMP,
				&rectangle3,
				Alpha,
				D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
				&Zoom);

		}
	}
	else {
		//D2D1_SIZE_F rtSize = pRT->GetSize();
		if (BMP != NULL)
		{
			//D2D1_SIZE_U sizeU = BMP->GetPixelSize();

			D2D1_RECT_F rectangle3 = D2D1::RectF(
				(float)X,
				(float)Y,
				(float)X + Width,
				(float)Y + Height
			);

			pRT->DrawBitmap(BMP, &rectangle3, Alpha);
		}


	}
}

void D2DDraw::ClearScr(const D2D1_COLOR_F COL) {
	pRT->Clear(COL);
}

D2DBitMap* D2DDraw::LoadPNG(LPCWSTR FileName, UINT destinationWidth, UINT destinationHeight)
{

	D2DBitMap* g_bitmap = NULL;


	IWICBitmapDecoder *bitmapdecoder = NULL;
	m_pWicImagingFactory->CreateDecoderFromFilename(FileName, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &bitmapdecoder);//  

	IWICBitmapFrameDecode  *pframe = NULL;
	bitmapdecoder->GetFrame(0, &pframe);

	IWICFormatConverter * fmtcovter = NULL;
	m_pWicImagingFactory->CreateFormatConverter(&fmtcovter);
	fmtcovter->Initialize(pframe, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	pRT->CreateBitmapFromWicBitmap(fmtcovter, NULL, &g_bitmap);

	fmtcovter->Release();
	pframe->Release();
	bitmapdecoder->Release();

	return g_bitmap;


}

D2DBitMap* D2DDraw::LoadPNGA(LPCWSTR FileName, UINT destinationWidth, UINT destinationHeight) 
{

	return NULL;
}

TextFormat* D2DDraw::LoadText(LPCWSTR Typeface, float Size, LPCWSTR Language)
{
	float Insize = Size * 96.0f / 72.0f;

	TextFormat* pITextFormat = NULL;

	pIDWriteFactory->CreateTextFormat(
		Typeface,
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		Size,
		Language,
		&pITextFormat
	);

	//ReleaseText();

	return pITextFormat;
}

void D2DDraw::D2DDrawText(D2D1::ColorF Color, RECT_F TextRC, LPCWSTR Text, TextFormat* Format)
{
	D2D1_RECT_F FRect;
	FRect = D2D1::RectF(TextRC.left, TextRC.top, TextRC.right, TextRC.bottom);

	pRT->CreateSolidColorBrush(
		Color,
		&pWriBrush
	);

	pRT->DrawText(
		Text,
		wcslen(Text),
		Format,
		FRect,
		pWriBrush
	);
}

void D2DDraw::ReleaseText()
{
	SafeRelease(pIWriBrush);
	SafeRelease(pIDWriteFactory);
	SafeRelease(pWriBrush);
}

/*void D2DDraw::SetTrans(float Trans)
{
	pRT->PushLayer(
		D2D1::LayerParameters(
			D2D1::InfiniteRect(),
			NULL,
			D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
			D2D1::IdentityMatrix(),
			Trans,
			NULL,
			D2D1_LAYER_OPTIONS_NONE
		),
		g_pLayer
	);

	TFEffect = true;
}*/

void D2DDraw::DrawRect(RECT_F reCt, D2D1::ColorF Color)
{
	D2D1_RECT_F Frc;

	Frc.top = reCt.top;
	Frc.bottom = reCt.bottom;
	Frc.left = reCt.left;
	Frc.right = reCt.right;

	pRT->CreateSolidColorBrush(
		Color,
		&pBrush
	);

	pRT->FillRectangle(Frc, rBrush);

}

BMPRect D2DDraw::GetBMPRect(D2DBitMap* bmp) {
	BMPRect re;
	re = bmp->GetPixelSize();
	return re;
}




