#include "Graphics.h"

bool Graphics::Initialize(HWND hWnd, int width, int height)
{
    _fpsTimer.Start();

    if (!this->InitD3D11(hWnd, width, height))
        return false;

    if (!this->InitPipeline())
        return false;

    if (!this->InitGraphicsD3D11())
        return false;

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.WantSaveIniSettings = false;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMinSize = ImVec2(300, 500);
    style.WindowTitleAlign = ImVec2(0.50f, 0.50f); // Title
    // style.WindowPadding = ImVec2(20.0f, 20.0f);
    // style.WindowRounding = 9.0f;
    // style.FrameRounding = 12.0f;
    // style.FramePadding = ImVec2(17.0f, 4.0f);
    // style.TabRounding = 9.0f;
    // style.GrabRounding = 10.0f;
    // style.GrabMinSize = 15.0f;
    // style.ScrollbarSize = 17.0f;
    // style.ItemSpacing = ImVec2(13.0f, 4.0f);
    // style.ItemInnerSpacing = ImVec2(10.0f, 8.0f);

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(_dev, _devcon);
    ImGui::StyleColorsDark();

    return true;
}

bool Graphics::InitD3D11(HWND hWnd, int width, int height)
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
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching

    HRESULT hr;   
    
    // Create device, device context and swap chain using the information in the scd struct
    hr = D3D11CreateDeviceAndSwapChain( NULL,
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

    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create device and swapchain.");
        return false;
    }

    // Get address of back buffer
    ID3D11Texture2D* pBackBuffer;
    _swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // Use the back buffer address to create the render target
    hr = _dev->CreateRenderTargetView(pBackBuffer, NULL, &_backbuffer);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create render target view.");
        return false;
    }

    // Set render target as the back buffer
    _devcon->OMSetRenderTargets(1, &_backbuffer, NULL);

    // Set viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = width;
    viewport.Height = height;

    _devcon->RSSetViewports(1, &viewport);

    // Create rasterizer state
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));

    rd.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    hr = _dev->CreateRasterizerState(&rd, &_rasterizerState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create rasterizer state.");
        return false;
    }

    return true;
}

bool Graphics::InitPipeline(void)
{
    // Load and compile the two shaders
    ID3D10Blob* VS, * PS;  // buffer with compiled code of the shader (COM obj)
    
    HRESULT hr;
    hr = D3DX11CompileFromFile(L"vertexshader.hlsl", 0, 0, "main", "vs_4_0", 0, 0, 0, &VS, 0, 0);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to compile vertex shader.");
        return false;
    }

    hr = D3DX11CompileFromFile(L"pixelshader.hlsl", 0, 0, "main", "ps_4_0", 0, 0, 0, &PS, 0, 0);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to compile pixel shader.");
        return false;
    }

    // Encapsulate shaders into shader objects
    hr = _dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &_pVS);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create vertex shader.");
        return false;
    }
    
    hr = _dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &_pPS);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create pixel shader.");
        return false;
    }

    // Creating input layout to let gpu organize data properly
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
    };

    hr = _dev->CreateInputLayout(ied, ARRAYSIZE(ied), VS->GetBufferPointer(), VS->GetBufferSize(), &_pLayout);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create Input layout.");
        return false;
    }

    return true;
}

bool Graphics::InitGraphicsD3D11(void)
{
    // create a square using the VERTEX struct
    Vertex v[] =
    {
        {-0.3f, 0.5f, 1.0f, 0.0f, 0.0f},   
        {0.3f, 0.5f, 0.0f, 1.0f, 0.0f},  
        {-0.3f, -0.5f, 0.0f, 0.0f, 1.0f},     
        {0.3f, -0.5f, 1.0f, 1.0f, 1.0f},   
    };

    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;                         // write access access by CPU and GPU
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(v);           // size is the VERTEX struct * 3
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;                // use as a vertex buffer
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;             // allow CPU to write in buffer

    D3D11_SUBRESOURCE_DATA vertexBufferData;
    ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
    vertexBufferData.pSysMem = v;

    HRESULT hr;
    hr = _dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &_pVBuffer);     // create the buffer
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create vertex buffer.");
        return false;
    }

    // Copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    _devcon->Map(_pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
    memcpy(ms.pData, v, sizeof(v));                 // copy the data
    _devcon->Unmap(_pVBuffer, NULL);        // unmap the buffer, allow the gpu to access the buffer

    return true;
}

void Graphics::RenderFrame(void)
{
    // Clear the back buffer to a color
    _devcon->ClearRenderTargetView(_backbuffer, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

    {
        _devcon->IASetInputLayout(_pLayout);

        // Tell Direct3D which type of primitive to use
        _devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        
        _devcon->RSSetState(_rasterizerState);

        // Activate shaders
        _devcon->VSSetShader(_pVS, 0, 0);
        _devcon->PSSetShader(_pPS, 0, 0);

        // Tell the GPU which vertices to read from when rendering
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        _devcon->IASetVertexBuffers(0, 1, &_pVBuffer, &stride, &offset);

        // Draw the vertex buffer to the back buffer
        _devcon->Draw(4, 0);    // draw x verticies, starting from vertex 0

        static int fpsCount = 0;
        fpsCount += 1;
        if (_fpsTimer.GetMillisecondElapsed() > 1000.0f)
        {
            fpsCount = 0;
            _fpsTimer.Restart();
        }
    }

    // Start ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Window");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Switch back buffer and front buffer
    _swapchain->Present(0, 0); // 1 for VSync
}

void Graphics::CleanD3D(void)
{
    _swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

    // close and release all existing COM objects
    if (_rasterizerState) _rasterizerState->Release();
    if (_pLayout) _pLayout->Release();
    if (_pVS) _pVS->Release();
    if (_pPS) _pPS->Release();
    if (_pVBuffer) _pVBuffer->Release();
    if (_swapchain) _swapchain->Release();
    if (_backbuffer) _backbuffer->Release();
    if (_dev) _dev->Release();
    if (_devcon) _devcon->Release();
}