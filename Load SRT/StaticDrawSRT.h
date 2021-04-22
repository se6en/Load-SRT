#pragma once

using namespace Microsoft::WRL;

class CStaticDrawSRT : public CStatic
{
   DECLARE_DYNAMIC(CStaticDrawSRT)
public:
   CStaticDrawSRT();
   virtual ~CStaticDrawSRT();

   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnPaint();

   virtual void PreSubclassWindow();

   DECLARE_MESSAGE_MAP()

public:
   void ShowSRTData(CString strStartTime, CString strEndTime, CString strContent);

private:
   HRESULT CreateDeviceIndependentResources();
   HRESULT CreateDeviceResources();

   HRESULT CreateSwapChainBitmap(ComPtr<IDXGISwapChain1>& pSwapChain, ComPtr<ID2D1DeviceContext1>& pContext, ComPtr<ID2D1Bitmap1>& pBitmap);

private:
   ComPtr<ID2D1Factory2>                    m_pD2DFactory;
   ComPtr<ID2D1Device1>                     m_pDevice;
   ComPtr<ID2D1DeviceContext1>              m_pD2DContext;
   ComPtr<IDXGISwapChain1>                  m_pSwapChain;
   ComPtr<ID2D1Bitmap1>                     m_pTargetBitmap;
   ComPtr<ID2D1HwndRenderTarget>            m_pRenderTarget;

   ComPtr<IDWriteFactory>                   m_pDWriteFactory;
   ComPtr<IDWriteTextFormat>                m_pTextFormat;
   ComPtr<IDWriteTextLayout>                m_pTextLayout;

};
