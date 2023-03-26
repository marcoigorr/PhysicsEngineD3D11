#include "Graphics.h"

bool Graphics::Initialize(HWND hWnd, int width, int height)
{
    _wWidth = width;
    _wHeight = height;

    _fpsTimer.Start();
    
    if (!this->InitD3D11(hWnd))
        return false;

    if (!this->InitPipeline())
        return false;

    if (!this->InitGraphicsD3D11())
        return false;

    if (!this->InitImGui(hWnd))
        return false;

    return true;
}

bool Graphics::InitImGui(HWND hWnd)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.WantSaveIniSettings = false;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMinSize = ImVec2(550, 200);
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

    if (!ImGui_ImplWin32_Init(hWnd))
    {
        ErrorLogger::Log("ImGui_ImplWin32_Init, failed to initialize ImGui.");
        return false;
    }

    if (!ImGui_ImplDX11_Init(_dev, _devcon))
    {
        ErrorLogger::Log("ImGui_ImplDX11_Init, failed to initialize ImGui.");
        return false;
    }

    return true;
}

bool Graphics::InitD3D11(HWND hWnd)
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
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;                                    // windowed/full-screen mode
    scd.BufferDesc.Width = _wWidth;
    scd.BufferDesc.Height = _wHeight;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching
    scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

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

    // Depth Stencil buffer
    D3D11_TEXTURE2D_DESC dsd;
    ZeroMemory(&dsd, sizeof(D3D11_TEXTURE2D_DESC));
    dsd.Width = _wWidth;
    dsd.Height = _wHeight;
    dsd.MipLevels = 1;
    dsd.ArraySize = 1;
    dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsd.SampleDesc.Count = 8;
    dsd.SampleDesc.Quality = 0;
    dsd.Usage = D3D11_USAGE_DEFAULT;
    dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    dsd.CPUAccessFlags = 0;
    dsd.MiscFlags = 0;

    hr = _dev->CreateTexture2D(&dsd, NULL, &_depthStencilBuffer);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create depth stencil buffer.");
        return false;
    }

    hr = _dev->CreateDepthStencilView(_depthStencilBuffer, NULL, &_depthStencilView);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create depth stencil view.");
        return false;
    }

    // Set render target as the back buffer
    _devcon->OMSetRenderTargets(1, &_backbuffer, _depthStencilView);

    // Create depth stencil state
    D3D11_DEPTH_STENCIL_DESC dsdesc;
    ZeroMemory(&dsdesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsdesc.DepthEnable = true;
    dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = _dev->CreateDepthStencilState(&dsdesc, &_depthStencilState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create depth stencil state.");
        return false;
    }

    // Set viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = _wWidth;
    viewport.Height = _wHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    _devcon->RSSetViewports(1, &viewport);

    // Create rasterizer state
    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
    rd.AntialiasedLineEnable = true;
    rd.MultisampleEnable = true;
    rd.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rd.FrontCounterClockwise = true;

    hr = _dev->CreateRasterizerState(&rd, &_rasterizerState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create rasterizer state.");
        return false;
    }

    // Create blend state
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA; 
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;

    hr = _dev->CreateBlendState(&blendDesc, &_blendState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create blend state.");
        return false;
    }

    _spriteBatch = std::make_unique<DirectX::SpriteBatch>(_devcon);
    _spriteFont = std::make_unique<DirectX::SpriteFont>(_dev, L"Data\\Fonts\\arial_14.spritefont");

    // Texture
    D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(D3D11_SAMPLER_DESC));
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // x coord on texture
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  // y
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;  // for 3d textures
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.MinLOD = 0;  // Level of detail
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    
    hr = _dev->CreateSamplerState(&sd, &_samplerState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create sampler state.");
        return false; 
    }

    return true;
}

bool Graphics::InitPipeline(void)
{
    // Load and compile the two shaders
    ID3D10Blob* VS, * PS;  // buffer with compiled code of the shader (COM obj)
    
    HRESULT hr;
    hr = D3DX11CompileFromFile(L"vertexshader.hlsl", 0, 0, "main", "vs_5_0", 0, 0, 0, &VS, 0, 0);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to compile vertex shader.");
        return false;
    }

    hr = D3DX11CompileFromFile(L"pixelshader.hlsl", 0, 0, "main", "ps_5_0", 0, 0, 0, &PS, 0, 0);
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
        {"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
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
    HRESULT hr;     

    // Load texture
    hr = D3DX11CreateShaderResourceViewFromFile(_dev, L"Data\\Textures\\particle.png", NULL, NULL, &_particleTexture, NULL);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create texture from file.");
        return false;
    }

    // Initialize contant buffer(s)
    hr = _cb_vs_vertexshader.Initialize(_dev, _devcon);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create constant buffer.");
        return false;
    } 
    
    hr = _cb_ps_pixelshader.Initialize(_dev, _devcon);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create constant buffer.");
        return false;
    } 

    // Initialize Entities
    if (!_entity[0].Initialize(_dev, _devcon, _particleTexture, _cb_vs_vertexshader, _cb_ps_pixelshader))
        return false;

    if (!_entity[1].Initialize(_dev, _devcon, _particleTexture, _cb_vs_vertexshader, _cb_ps_pixelshader))
        return false;

    _entity[1].SetPosition(20.0f, 20.0f, 100.0f);

    _camera.SetProjectionValues(90.0f, static_cast<float>(_wWidth) / static_cast<float>(_wHeight), 0.1f, 1000.0f);

    return true;
}

void Graphics::RenderFrame(void)
{
    // Clear the back buffer to a color
    _devcon->ClearRenderTargetView(_backbuffer, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

    // Refresh depth stencil view
    _devcon->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };
    _devcon->OMSetBlendState(_blendState, blendFactor, 0xFFFFFFFF);

    _devcon->IASetInputLayout(_pLayout);
    _devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _devcon->RSSetState(_rasterizerState);
    _devcon->OMSetDepthStencilState(_depthStencilState, 0);
    _devcon->PSSetSamplers(0, 1, &_samplerState);
    _devcon->VSSetShader(_pVS, 0, 0);
    _devcon->PSSetShader(_pPS, 0, 0);

    // Entity draw and manipulation
    static XMFLOAT3 cameraPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    static XMFLOAT3 entityPos = XMFLOAT3(0.0f, 0.0f, 100.0f); // second entity, the first one is static
    static bool isEditing = false;
    
    _camera.SetPosition(cameraPos);

    for (int i = 0; i < ARRAYSIZE(_entity); i++)
    {
        if (isEditing)
            _entity[1].SetPosition(entityPos);
        _entity[i].Draw(_camera.GetViewMatrix() * _camera.GetProjectionMatrix());

        entityPos = _entity[1].GetPositionFloat3();
    }        
   
    // Text / FPS
    static int fpsCount = 0;
    static std::string fpsString = "FPS: 0";
    fpsCount += 1;
    if (_fpsTimer.GetMillisecondElapsed() > 1000.0f)
    {
        fpsString = "FPS: " + std::to_string(fpsCount);
        fpsCount = 0;
        _fpsTimer.Restart();
    }

    // Start ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    {
        // ImGui::SetNextWindowSize(ImVec2(500, 200));
        ImGui::Begin("Window");
        {
            ImGui::Text(fpsString.c_str());
            static float* cam[3] = { &cameraPos.x, &cameraPos.y, &cameraPos.z };
            ImGui::SliderFloat3("Camera Position (x, y, z)", *cam, -100.0f, 100.0f, "%0.1f");
            if (ImGui::Button("RESET CAMERA", { 100.0f,20.0f }))
            {
                cameraPos.x = 0.0f;
                cameraPos.y = 0.0f;
                cameraPos.z = 0.0f;
            }

            ImGui::Spacing();
            
            ImGui::Checkbox("Enable edit", &isEditing);
            if (isEditing)
            {
                static float* ent[3] = { &entityPos.x, &entityPos.y, &entityPos.z };
                ImGui::SliderFloat3("Entity Position (x, y, z)", *ent, -100.0f, 100.0f, "%0.1f", 0);
                if (ImGui::Button("RESET ENTITY", { 100.0f,20.0f }))
                {
                    entityPos.x = 0.0f;
                    entityPos.y = 20.0f;
                    entityPos.z = 100.0f;
                }
            }           

        } ImGui::End();           
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Font Render
    _spriteBatch->Begin();
    _spriteFont->DrawString(_spriteBatch.get(), fpsString.c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
    _spriteBatch->End();

    // Switch back buffer and front buffer
    _swapchain->Present(0, 0); // 1 for VSync
}

void Graphics::CleanD3D(void)
{
    _swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

    // close and release all existing COM objects
    if (_particleTexture) _particleTexture->Release();
    if (&_entity[0]) _entity[0].Release();
    if (&_entity[1]) _entity[1].Release();
    if (_spriteBatch) _spriteBatch.release();
    if (_spriteFont) _spriteFont.release();
    if (_depthStencilState) _depthStencilState->Release();
    if (_depthStencilView) _depthStencilView->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();
    if (_blendState) _blendState->Release();
    if (_rasterizerState) _rasterizerState->Release();
    if (_pLayout) _pLayout->Release();
    if (_pVS) _pVS->Release();
    if (_pPS) _pPS->Release();
    if (_samplerState) _samplerState->Release();
    if (_cb_ps_pixelshader.GetAddressOf()) _cb_ps_pixelshader.Release();
    if (_cb_vs_vertexshader.GetAddressOf()) _cb_vs_vertexshader.Release();    
    if (_swapchain) _swapchain->Release();
    if (_backbuffer) _backbuffer->Release();
    if (_dev) _dev->Release();
    if (_devcon) _devcon->Release();
}
