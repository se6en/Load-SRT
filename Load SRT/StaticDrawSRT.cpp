#include "stdafx.h"
#include "StaticDrawSRT.h"

IMPLEMENT_DYNAMIC(CStaticDrawSRT, CStatic)

CStaticDrawSRT::CStaticDrawSRT()
{
   m_pD2DFactory = nullptr;
   m_pDevice = nullptr;
   m_pD2DContext = nullptr;
   m_pSwapChain = nullptr;
   m_pTargetBitmap = nullptr;

   m_pDWriteFactory = nullptr;
   m_pRenderTarget = nullptr;
   m_pTextFormat = nullptr;
   m_pTextLayout = nullptr;
}

CStaticDrawSRT::~CStaticDrawSRT()
{

}

BEGIN_MESSAGE_MAP(CStaticDrawSRT, CStatic)
   ON_WM_SIZE()
   ON_WM_PAINT()
END_MESSAGE_MAP()

void CStaticDrawSRT::OnSize(UINT nType, int cx, int cy)
{
   if (m_pSwapChain == nullptr || m_pD2DContext == nullptr)
   {
      return;
   }

   ID2D1Image* pImage = nullptr;
   m_pD2DContext->GetTarget(&pImage);
   m_pD2DContext->SetTarget(nullptr);

   if (pImage != nullptr)
   {
      pImage->Release();
   }

   m_pTargetBitmap = nullptr;

   IDXGISurface* pBuffer = nullptr;
   m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBuffer));
   if (pBuffer != nullptr)
   {
      int i = pBuffer->Release();
      while (i > 0)
      {
         i = pBuffer->Release();
      }
   }

   pBuffer = nullptr;
   m_pSwapChain->GetBuffer(1, IID_PPV_ARGS(&pBuffer));
   if (pBuffer != nullptr)
   {
      int i = pBuffer->Release();
      while (i > 0)
      {
         i = pBuffer->Release();
      }
   }

   HRESULT hr = m_pSwapChain->ResizeBuffers(0, cx, cy, DXGI_FORMAT_UNKNOWN, 0);
   
   if (SUCCEEDED(hr))
   {
      hr = CreateSwapChainBitmap(m_pSwapChain, m_pD2DContext, m_pTargetBitmap);
   }

   if (SUCCEEDED(hr))
   {
      m_pD2DContext->SetTarget(m_pTargetBitmap.Get());
   }

}

void CStaticDrawSRT::OnPaint()
{
   CPaintDC dc(this);

   if (FAILED(CreateDeviceResources()))
   {
      return;
   }

   m_pD2DContext->SetTarget(m_pTargetBitmap.Get());
   m_pD2DContext->BeginDraw();
   m_pD2DContext->Clear();

   ComPtr<ID2D1SolidColorBrush> pBgBrush = nullptr;
   HRESULT hr = m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(0.f, 0.f, 0.f), &pBgBrush);

   if(FAILED(hr))
   { 
      return;
   }

   CRect rcClient;
   GetClientRect(rcClient);

   D2D1_RECT_F rcBkg = { (FLOAT)rcClient.left, (FLOAT)rcClient.top, (FLOAT)rcClient.right, (FLOAT)rcClient.bottom };
   m_pD2DContext->FillRectangle(rcBkg, pBgBrush.Get());

   if (m_pTextLayout != nullptr)
   {
      ComPtr<ID2D1SolidColorBrush> pTextBrush = nullptr;
      hr = m_pD2DContext->CreateSolidColorBrush(D2D1::ColorF(1.F, 1.F, 1.F), &pTextBrush);

      DWRITE_TEXT_METRICS textMetrics;
      m_pTextLayout->GetMetrics(&textMetrics);

      D2D1_POINT_2F ptTextOrigin = D2D1::Point2F((float)0.f, (float)-textMetrics.top);
      m_pD2DContext->DrawTextLayout(ptTextOrigin, m_pTextLayout.Get(), pTextBrush.Get(), D2D1_DRAW_TEXT_OPTIONS_NO_SNAP);
   }

   m_pD2DContext->EndDraw();

   DXGI_PRESENT_PARAMETERS parameters = { 0 };
   parameters.DirtyRectsCount = 0;
   parameters.pDirtyRects = nullptr;
   parameters.pScrollRect = nullptr;
   parameters.pScrollOffset = nullptr;
   hr = m_pSwapChain->Present1(1, 0, &parameters);

}

void CStaticDrawSRT::PreSubclassWindow()
{
   CreateDeviceIndependentResources();

   CStatic::PreSubclassWindow();
}

void CStaticDrawSRT::ShowSRTData(CString strStartTime, CString strEndTime, CString strContent)
{
   if (m_pDWriteFactory == nullptr)
   {
      return;
   }

   CRect rcClient;
   GetClientRect(rcClient);

   if (rcClient.IsRectEmpty())
   {
      return;
   }

   CString strData = strStartTime + _T("\n");
   strData += strEndTime;
   strData += _T("\n");
   strData += strContent;

   ComPtr<IDWriteTextFormat> pD2DTextFormat = nullptr;
   HRESULT hr = m_pDWriteFactory->CreateTextFormat(
      _T("Arial"),
      NULL,
      DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      50.F,
      L"",
      &pD2DTextFormat
   );

   if (FAILED(hr))
   {
      return;
   }

   pD2DTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_EMERGENCY_BREAK);
   pD2DTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
   
   hr = m_pDWriteFactory->CreateTextLayout(
      strData,
      strData.GetLength(),
      pD2DTextFormat.Get(),
      (FLOAT)rcClient.Width(),
      (FLOAT)rcClient.Height(),
      &m_pTextLayout
   );

   if (FAILED(hr))
   {
      return;
   }

   m_pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

   Invalidate();
   UpdateWindow();
}

HRESULT CStaticDrawSRT::CreateDeviceIndependentResources()
{
   IDXGIAdapter* pDxgiAdapter = nullptr;
   ID3D11Device* pD3D11Device = nullptr;
   ID3D11DeviceContext* pD3D11DeviceContext = nullptr;
   IDXGIDevice1* pDxgiDevice = nullptr;
   IDXGIFactory2* pDxgiFactory = nullptr;
   IDXGISurface* pDxgiBackBuffer = nullptr;

   UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
   creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

   D3D_FEATURE_LEVEL featuresLevels[] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0
   };

   D3D_FEATURE_LEVEL featureLevel;

   HRESULT hr = D3D11CreateDevice(pDxgiAdapter, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featuresLevels, sizeof(featuresLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &pD3D11Device, &featureLevel, &pD3D11DeviceContext);

   if (SUCCEEDED(hr))
   {
      hr = pD3D11Device->QueryInterface(__uuidof(IDXGIDevice1), (void **)&pDxgiDevice);
   }

   if (SUCCEEDED(hr))
   {
      hr = pDxgiDevice->GetAdapter(&pDxgiAdapter);
   }

   if (SUCCEEDED(hr))
   {
      hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory));
   }

   if (SUCCEEDED(hr))
   {
      DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
      swapChainDesc.Height = 0;
      swapChainDesc.Width = 0;
      swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
      swapChainDesc.BufferCount = 2;
      swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swapChainDesc.Flags = 0;
      swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      swapChainDesc.SampleDesc.Count = 1;
      swapChainDesc.SampleDesc.Quality = 0;
      swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
      swapChainDesc.Stereo = FALSE;
      swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

      hr = pDxgiFactory->CreateSwapChainForHwnd(pD3D11Device, m_hWnd, &swapChainDesc, nullptr, nullptr, &m_pSwapChain);
   }

   if (SUCCEEDED(hr))
   {
      hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &m_pD2DFactory);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_pD2DFactory->CreateDevice(pDxgiDevice, &m_pDevice);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_pDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2DContext);
      m_pD2DContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
   }

   if (SUCCEEDED(hr))
   {
      hr = CreateSwapChainBitmap(m_pSwapChain, m_pD2DContext, m_pTargetBitmap);
   }

   if (SUCCEEDED(hr))
   {
      hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_pDWriteFactory);
   }

   return hr;
}

HRESULT CStaticDrawSRT::CreateDeviceResources()
{
   if (m_pRenderTarget != nullptr)
   {
      return S_OK;
   }

   CRect rcClient;
   GetClientRect(rcClient);

   D2D1_SIZE_U szRenderTarget = D2D1::SizeU(rcClient.Width(), rcClient.Height());

   D2D1_RENDER_TARGET_PROPERTIES renderTargetProperty = D2D1::RenderTargetProperties(
      /*D2D1_RENDER_TARGET_TYPE_DEFAULT*/D2D1_RENDER_TARGET_TYPE_HARDWARE,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
      96.0,
      96.0
   );

   HRESULT hr = m_pD2DFactory->CreateHwndRenderTarget(
      /*D2D1::RenderTargetProperties()*/renderTargetProperty,
      D2D1::HwndRenderTargetProperties(GetSafeHwnd(), szRenderTarget, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
      &m_pRenderTarget
   );

   return hr;
}

HRESULT CStaticDrawSRT::CreateSwapChainBitmap(ComPtr<IDXGISwapChain1>& pSwapChain, ComPtr<ID2D1DeviceContext1>& pContext, ComPtr<ID2D1Bitmap1>& pBitmap)
{
   ComPtr<IDXGISurface> dxgiBackBuffer = nullptr;
   HRESULT hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));

   if (FAILED(hr))
   {
      return S_FALSE;
   }

   D2D1_BITMAP_PROPERTIES1 bitmapProperties =
      D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
         D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
         96.F,
         96.F);

   pBitmap = nullptr;

   hr = pContext->CreateBitmapFromDxgiSurface(
      dxgiBackBuffer.Get(),
      &bitmapProperties,
      &pBitmap);

   return hr;
}

