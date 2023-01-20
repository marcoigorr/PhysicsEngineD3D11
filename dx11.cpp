#include "pch.h"
#include "dx11.h"

void Direct3D11::InitD3D(HWND hWnd)
{
    // Struct hold information about swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    // Set scd struct to NULL
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    // Fill the swap chain description struct
    scd.BufferCount = 1;                                    // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
    scd.OutputWindow = hWnd;                                // the window to be used
    scd.SampleDesc.Count = 8;                               // MSAA (Anti-Alias)
    scd.Windowed = TRUE;                                    // windowed/full-screen mode
    scd.BufferDesc.Width = SCREEN_WIDTH;
    scd.BufferDesc.Height = SCREEN_HEIGHT;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching

    // Create device, device context and swap chain using the information in the scd struct
    D3D11CreateDeviceAndSwapChain(NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        NULL,
        NULL,
        NULL,
        D3D11_SDK_VERSION,
        &scd,
        &_swapchain,
        &_dev,
        NULL,
        &_devcon);

    // Get address of back buffer
    ID3D11Texture2D* pBackBuffer;
    _swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // Use the back buffer address to create the render target
    _dev->CreateRenderTargetView(pBackBuffer, NULL, &_backbuffer);
    pBackBuffer->Release();

    // Set render target as the back buffer
    _devcon->OMSetRenderTargets(1, &_backbuffer, NULL);

    // Set viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = SCREEN_WIDTH;
    viewport.Height = SCREEN_HEIGHT;

    _devcon->RSSetViewports(1, &viewport);
}

void Direct3D11::CleanD3D(void)
{
    _swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

    // close and release all existing COM objects
    _pVS->Release();
    _pPS->Release();
    _swapchain->Release();
    _backbuffer->Release();
    _dev->Release();
    _devcon->Release();
}

void Direct3D11::RenderFrame(void)
{
    // Clear the back buffer to a color
    _devcon->ClearRenderTargetView(_backbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));

    // render here

    // Switch back buffer and front buffer
    _swapchain->Present(0, 0);
}

void Direct3D11::InitPipeline(void)
{
    // Load and compile the two shaders
    ID3D10Blob* VS, * PS;  // buffer with compiled code of the shader (COM obj)
    D3DX11CompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);
    D3DX11CompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);

    // Encapsulate shaders into shader objects
    _dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &_pVS);
    _dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &_pPS);

    // Activate shaders
    _devcon->VSSetShader(_pVS, 0, 0);
    _devcon->PSSetShader(_pPS, 0, 0);
}

Direct3D11* d3d11 = new Direct3D11();