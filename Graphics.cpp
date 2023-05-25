#include "Graphics.h"

#define e 2.71828182845904523536
#define PI 3.14159265
#define SUN_MASS 1.988435e30
#define PARSEC_IN_METER 3.08567758129e16

bool Graphics::Initialize(HWND hWnd, int width, int height)
{
    _INIFile = new mINI::INIFile("config.ini");

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
    if (!_imgui->Initialize(hWnd, _dev, _devcon))
        return false;
    return true;
}

bool Graphics::InitD3D11(HWND hWnd)
{
    HRESULT hr;

    // Struct hold information about swap chain
    DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    
    // Fill the swap chain description struct
    // Note: If multi-sample antialiasing is being used, all bound render targets and depth buffers must have the same sample counts and quality levels.
    scd.BufferCount = 1;                                    // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
    scd.OutputWindow = hWnd;                                // the window to be used
    scd.SampleDesc.Count = 1;                               // MSAA (Anti-Alias)
    scd.SampleDesc.Quality = 0;                             // The valid range is between zero and one less than the level returned by ID3D11Device::CheckMultisampleQualityLevels
    scd.Windowed = TRUE;                                    // windowed/full-screen mode
    scd.BufferDesc.Width = _wWidth;
    scd.BufferDesc.Height = _wHeight;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching
    scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    
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
    ID3D11Texture2D* pBackBuffer = nullptr;
    _swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    // Use the back buffer address to create the render target
    hr = _dev->CreateRenderTargetView(pBackBuffer, nullptr, &_backbuffer);
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
    dsd.SampleDesc.Count = 1;
    dsd.SampleDesc.Quality = 0;
    dsd.Usage = D3D11_USAGE_DEFAULT;
    dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    dsd.CPUAccessFlags = 0;
    dsd.MiscFlags = 0;

    // Create the texture for the depth buffer using the filled out description
    hr = _dev->CreateTexture2D(&dsd, NULL, &_depthStencilBuffer);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create depth stencil buffer.");
        return false;
    }

    // Create depth stencil state
    D3D11_DEPTH_STENCIL_DESC dsdesc;
    ZeroMemory(&dsdesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsdesc.DepthEnable = true;
    dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    dsdesc.StencilEnable = true;
    dsdesc.StencilReadMask = 0xFF;
    dsdesc.StencilWriteMask = 0xFF;

    //stencil operations if pixel is front-facing
    dsdesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsdesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsdesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsdesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    //stencil operations if pixel is back-facing
    dsdesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsdesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsdesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsdesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = _dev->CreateDepthStencilState(&dsdesc, &_depthStencilState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create depth stencil state.");
        return false;
    }

    // Initailze and create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    hr = _dev->CreateDepthStencilView(_depthStencilBuffer, &depthStencilViewDesc, &_depthStencilView);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create depth stencil view.");
        return false;
    }

    // Bind the render target view and depth stencil buffer to the output render pipeline
    _devcon->OMSetRenderTargets(1, &_backbuffer, _depthStencilView);

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
    rd.MultisampleEnable = false;
    rd.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rd.FrontCounterClockwise = false;

    hr = _dev->CreateRasterizerState(&rd, &_rasterizerState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create rasterizer state.");
        return false;
    }

    // Setup Blend State for transperency
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    blendDesc.AlphaToCoverageEnable = false;  // true if multi-sample renders
    blendDesc.RenderTarget[0] = rtbd;

    hr = _dev->CreateBlendState(&blendDesc, &_blendState);
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create blend state.");
        return false;
    }

    _spriteBatch = std::make_unique<DirectX::SpriteBatch>(_devcon);
    _spriteFont = std::make_unique<DirectX::SpriteFont>(_dev, L"Data\\Fonts\\arial_11.spritefont");

    // Texture sampler
    D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(D3D11_SAMPLER_DESC));
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  // x coord on texture
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  // y
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;  // for 3d textures
    sd.MipLODBias = 0;
    sd.MaxAnisotropy = 8;
    sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
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
    std::wstring shaderfolder = L"";    

#pragma region DetermineShaderPath
    
    if (IsDebuggerPresent())
    {
#ifdef _DEBUG // Debug Mode
        shaderfolder = L".\\x64\\Debug\\";
#else // Release Mode
        shaderfolder = L".\\x64\\Release\\";
#endif
    }

    // Input layout to let gpu organize data properly
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
        {"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
    };

    UINT numElements = ARRAYSIZE(ied);

    // Vertex Shader    
    if (!_pVS.Initialize(_dev, shaderfolder + L"vertexshader.cso", ied, numElements))
        return false;
    
    // Pixel Shader
    if (!_pPS.Initialize(_dev, shaderfolder + L"pixelshader.cso"))
        return false;       

    return true;
}

bool Graphics::InitGraphicsD3D11(void)
{
    HRESULT hr;     

    // Load image and create texture
    hr = D3DX11CreateShaderResourceViewFromFile(_dev, L"Data\\Textures\\particle.png", NULL, NULL, &_imageShaderResourceView, NULL);
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

    _camera.SetProjectionValues(90.0f, static_cast<float>(_wWidth) / static_cast<float>(_wHeight), 0.1f, 1000.0f);
    _cameraPos = _camera.GetDefPosition();

    float initialRange = 600.0f;
    _qtRoot = new QuadTreeNode(XMFLOAT2(-initialRange, initialRange), XMFLOAT2(initialRange, -initialRange), nullptr);

    return true;
}

Camera& Graphics::GetCamera()
{
    return _camera;
}

void Graphics::RenderFrame(void) 
{
    // Clear the back buffer to a color
    static float _bgCcolor[3] = { 0.02,0.02,0.032 };
    _devcon->ClearRenderTargetView(_backbuffer, D3DXCOLOR(_bgCcolor[0], _bgCcolor[1], _bgCcolor[2], 1.0f));

    // Refresh depth stencil view
    _devcon->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    constexpr float blendFactor[] = { 0, 0, 0, 0 };
    _devcon->OMSetBlendState(_blendState, blendFactor, 0xffffffff);

    _devcon->IASetInputLayout(_pVS.GetInputLayout());
    _devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _devcon->RSSetState(_rasterizerState);
    _devcon->OMSetDepthStencilState(_depthStencilState, 1);
    _devcon->PSSetSamplers(0, 1, &_samplerState);
    _devcon->VSSetShader(_pVS.GetShader(), nullptr, 0);
    _devcon->PSSetShader(_pPS.GetShader(), nullptr, 0);

    // Camera
    if (!_cameraTracking)
        _camera.SetPosition(_cameraPos);

    static float ps_mods_color[3] = { 1.000000, 0.718000, 0.781000 };

    // If there is at least a particle in the quadtree
    if (_particles.size())
    {
        // Reset and calculate new min/max (bounding box)
        XMFLOAT2 center = _qtRoot->GetCenterOfMass();
        _qtRoot->Reset(XMFLOAT2(center.x - _qtRoot->s_range, center.y + _qtRoot->s_range), XMFLOAT2(center.x + _qtRoot->s_range, center.y - _qtRoot->s_range));

        // Insert particles in the quadtree
        for (Entity* p : _particles)
        {
            _qtRoot->Insert(p, 0);
        }

        // Compute mass
        _qtRoot->ComputeMassDistribution();

        // Draw particles
        // _qtRoot->DrawEntities(_camera.GetViewMatrix() * _camera.GetProjectionMatrix());

        for (Entity* p : _particles)
        {
            if (_enableColors)
                p->SetColorModifiers(ps_mods_color[0], ps_mods_color[1], ps_mods_color[2]);

            p->Draw(_camera.GetViewMatrix() * _camera.GetProjectionMatrix());
        }

        if (_cameraTracking)
        {
            _camera.Track(center.x, center.y, _cameraPos.z);
            _cameraPos = _camera.GetPositionFloat3();
        }            
    }   

    // Text / FPS
    fpsCount += 1;
    if (_fpsTimer.GetMillisecondElapsed() > 1000.0f)
    {
        fpsString = "FPS: " + std::to_string(fpsCount);
        fpsCount = 0;
        _fpsTimer.Restart();
    }

    // Start ImGui
    _imgui->BeginRender();
    {
        ImGui::Begin("Physics Engine", nullptr, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            // File menu
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save Settings", "Ctrl+S"))
                {
                    _INIFile->write(_INIData); // save to config.ini
                }

                ImGui::EndMenu();
            }

            // Level menu
            if (ImGui::BeginMenu("Level"))
            {
                if (ImGui::MenuItem("Clear level", "Ctrl+R")) 
                { 
                    for (Entity* p : _particles)
                        p->Release();
                    if (_particles.size()) _particles.clear();

                    _pause = true;

                    if (_qtRoot->GetNum())
                        _qtRoot->Reset(XMFLOAT2(_qtRoot->s_range, _qtRoot->s_range), XMFLOAT2(_qtRoot->s_range, _qtRoot->s_range));
                }

                ImGui::EndMenu();
            }

            // Pause engine update
            ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 30);
            ImGui::Checkbox("Pause", &_pause);

            ImGui::Spacing();

            // Fps string
            ImGui::Text(fpsString.c_str());

            ImGui::EndMenuBar();
        }  

        // Scene configuration
        if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_FramePadding))
        {
            if (ImGui::BeginChild("scene conf", ImVec2(0, 150), true))
            {
                // Background color
                ImGui::ColorEdit3("Background", _bgCcolor);
                _INIData["Window"]["Backgroud"] = std::to_string(_bgCcolor[0]) + ", " + std::to_string(_bgCcolor[1]) + ", " + std::to_string(_bgCcolor[2]);

                // Pixel shader modifiers
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
                if (!_enableColors)
                {
                    if (ImGui::Button("ENABLE PARTICLES COLOR"))
                    {
                        _enableColors = true;
                        for (Entity* p : _particles)
                            p->SetColorFeature(true);
                    }
                }
                   
                if (_enableColors)
                {
                    if (ImGui::Button("DISABLE PARTICLES COLOR"))
                    {
                        _enableColors = false;
                        for (Entity* p : _particles)
                            p->SetColorFeature(false);
                    }

                    ImGui::SliderFloat3("ParticleColorMod", ps_mods_color, -1.0f, 2.0f);
                }
                _INIData["pixel shader"]["Color Mod"] = std::to_string(ps_mods_color[0]) + ", " + std::to_string(ps_mods_color[1]) + ", " + std::to_string(ps_mods_color[2]);
            }
            ImGui::EndChild();
        }

        // Camera transforms
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_FramePadding))
        {
            if (ImGui::BeginChild("camera conf", ImVec2(0, 150), true))
            {
                ImGui::DragFloat("X", &_cameraPos.x, 0.1f);
                ImGui::DragFloat("Y", &_cameraPos.y, 0.1f);
                ImGui::DragFloat("Z", &_cameraPos.z, 0.1f);

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

                if (ImGui::Button("Reset position"))
                {
                    _cameraPos = _camera.GetDefPosition();
                }

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
                
                ImGui::Checkbox(" Camera tracking", &_cameraTracking);
            }
            ImGui::EndChild();
        }

        // Level configuration
        if (ImGui::CollapsingHeader("Spawn", ImGuiTreeNodeFlags_FramePadding))
        {            
            static const char* items[] = { "Spiral Galaxy", "Galaxy Collision"};
            static int current_item = NULL;

            static int N(0);

            ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 10, ImGui::GetCursorPosY() + 10));
            ImGui::TextWrapped("Simulation selection");
            if (ImGui::BeginChild("Spawn", ImVec2(0, 250), true))
            {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
                if (ImGui::BeginCombo("##combo", items[current_item])) 
                {
                    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                    {
                        bool is_selected = (current_item == n); 
                        if (ImGui::Selectable(items[n], is_selected))
                            current_item = n;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                ImGui::InputInt("N (bodies)", &N, 1000);

                if (ImGui::Button("Spawn"))
                {
                    if (current_item == 0)
                        this->SpiralGalaxy(N);

                    else if (current_item == 1)
                        this->GalaxyCollision(N);
                }
            }
            ImGui::EndChild();
        }

        ImGui::End();
    }
    _imgui->EndRender();

    // Font Render
    _spriteBatch->Begin();
    {
        _spriteFont->DrawString(_spriteBatch.get(), fpsString.c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
        _spriteFont->DrawString(_spriteBatch.get(), ("Theta: " + std::to_string(_qtRoot->GetTheta())).c_str(), DirectX::XMFLOAT2(0, 20), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
        _spriteFont->DrawString(_spriteBatch.get(), ("N: " + std::to_string(_qtRoot->GetNum())).c_str(), DirectX::XMFLOAT2(0, 40), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
        _spriteFont->DrawString(_spriteBatch.get(), ("Min (x, y): " + std::to_string((int)_qtRoot->GetMin().x) + ", " + std::to_string((int)_qtRoot->GetMin().y)).c_str(), DirectX::XMFLOAT2(0, 60), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
        _spriteFont->DrawString(_spriteBatch.get(), ("Max (x, y): " + std::to_string((int)_qtRoot->GetMax().x) + ", " + std::to_string((int)_qtRoot->GetMax().y)).c_str(), DirectX::XMFLOAT2(0, 80), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
        _spriteFont->DrawString(_spriteBatch.get(), ("Center of Mass (x, y): " + std::to_string(_qtRoot->GetCenterOfMass().x) + ", " + std::to_string(_qtRoot->GetCenterOfMass().y)).c_str(), DirectX::XMFLOAT2(0, 100), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
    }
    _spriteBatch->End();

    // Switch back buffer and front buffer
    _swapchain->Present(1, 0); // 1 for VSync
}

void Graphics::CleanD3D(void)
{
    _imgui->ShutDown();

    _swapchain->SetFullscreenState(FALSE, NULL);  // switch to windowed mode

    // Close and release all existing COM objects
    if (_particles.size())
        for (int i = 0; i < _particles.size(); i++)
            if (_particles[i]) _particles[i]->Release();

    if (_imageShaderResourceView) _imageShaderResourceView->Release();
    if (_spriteBatch) _spriteBatch.release();
    if (_spriteFont) _spriteFont.release();
    if (_depthStencilState) _depthStencilState->Release();
    if (_depthStencilView) _depthStencilView->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();
    if (_blendState) _blendState->Release();
    if (_rasterizerState) _rasterizerState->Release();
    if (&_pVS) _pVS.Release();
    if (&_pPS) _pPS.Release();
    if (_samplerState) _samplerState->Release();
    if (_cb_ps_pixelshader.GetAddressOf()) _cb_ps_pixelshader.Release();
    if (_cb_vs_vertexshader.GetAddressOf()) _cb_vs_vertexshader.Release();    
    if (_swapchain) _swapchain->Release();
    if (_backbuffer) _backbuffer->Release();
    if (_dev) _dev->Release();
    if (_devcon) _devcon->Release();
}

// Necessary velocity to escape M1 gravity
XMFLOAT2 Graphics::GetOrbitalVelocity(const Entity* p1, const Entity* p2)
{
    double x1 = p1->GetPositionFloat3().x,
           y1 = p1->GetPositionFloat3().y,
           M1 = p1->GetMass();
    double x2 = p2->GetPositionFloat3().x,
           y2 = p2->GetPositionFloat3().y;

    double xDistance = x1 - x2,
           yDistance = y1 - y2;

    double r = sqrt((xDistance * xDistance) + (yDistance * yDistance)); // distance between p1 and p2

    double v = sqrt(_qtRoot->gamma_1 * M1 / r); // orbital velocity

    double vx = (yDistance / r) * v,
           vy = (-xDistance / r) * v; 

    return XMFLOAT2(vx, vy);
}

void Graphics::SpiralGalaxy(int N)
{   
    srand(static_cast<unsigned>(time(0)));

    float spawn_range = 100.0f;
    float particle_radius = 2.0f;
    double particle_mass = 1.9722e11;
    XMFLOAT2 galaxy_center = { 50.0f,50.0f };
    XMFLOAT2 velocity = { 0.0f,0.0f };

    // Create a black hole
    Entity* blackHole = new Entity();
    blackHole->Create(0.5f, 1.988435e16, _imageShaderResourceView, XMFLOAT3(galaxy_center.x, galaxy_center.y, 0.0f), XMFLOAT2(0.0f, 0.0f));
    blackHole->Initialize(_dev, _devcon, _cb_vs_vertexshader, _cb_ps_pixelshader);
    _particles.push_back(blackHole);

    for (int i = 0; i < N; i++)
    {
        float x(0), y(0), r(0);
        r = spawn_range * sqrt((double)rand() / RAND_MAX) + 20.0f;

        float theta = ((double)rand() / RAND_MAX) * 2 * PI;
        x = galaxy_center.x + r * cos(theta);
        y = galaxy_center.y + r * sin(theta);

        Entity* newParticle = new Entity();
        newParticle->Create(particle_radius, particle_mass * ((double)rand() / RAND_MAX), _imageShaderResourceView, XMFLOAT3(x, y, 0.0f), XMFLOAT2(0.0f,0.0f));
        newParticle->Initialize(_dev, _devcon, _cb_vs_vertexshader, _cb_ps_pixelshader);
        _particles.push_back(newParticle);

        velocity = GetOrbitalVelocity(blackHole, newParticle);
        newParticle->SetVelocity(velocity);
    }
}

void Graphics::GalaxyCollision(int N)
{
    srand(static_cast<unsigned>(time(0)));

    int N2 = N * 0.2;
    int N1 = N - N2;

    float spawn_range = 100.0f;
    float particle_radius = 2.0f;
    double particle_mass = 5.9722e11;
    XMFLOAT2 galaxy_center = { 0.0f,0.0f };
    XMFLOAT2 velocity = { 0.0f,0.0f };

    // Galaxy 1 black hole
    Entity* blackHole1 = new Entity();
    blackHole1->Create(0.5f, 3.988435e16, _imageShaderResourceView, XMFLOAT3(galaxy_center.x, galaxy_center.y, 0.0f), XMFLOAT2(0.0f, 0.0f));
    blackHole1->Initialize(_dev, _devcon, _cb_vs_vertexshader, _cb_ps_pixelshader);
    _particles.push_back(blackHole1);

    for (int i = 0; i < N1; i++)
    {
        float x(0), y(0), r(0);
        r = spawn_range * sqrt((double)rand() / RAND_MAX) + 20.0f;

        float theta = ((double)rand() / RAND_MAX) * 2 * PI;
        x = galaxy_center.x + r * cos(theta);
        y = galaxy_center.y + r * sin(theta);

        Entity* newParticle = new Entity();
        newParticle->Create(particle_radius, particle_mass * ((double)rand() / RAND_MAX), _imageShaderResourceView, XMFLOAT3(x, y, 0.0f), XMFLOAT2(0.0f, 0.0f));
        newParticle->Initialize(_dev, _devcon, _cb_vs_vertexshader, _cb_ps_pixelshader);
        _particles.push_back(newParticle);
        velocity = GetOrbitalVelocity(blackHole1, newParticle);
        newParticle->SetVelocity(velocity);
    }

    spawn_range = 70.0f;
    galaxy_center = { 200.0f,200.0f };

    // Galaxy 2 black hole
    Entity* blackHole2 = new Entity();
    blackHole2->Create(0.5f, 1.988435e16, _imageShaderResourceView, XMFLOAT3(galaxy_center.x, galaxy_center.y, 0.0f), XMFLOAT2(0.0f, 0.0f));
    blackHole2->Initialize(_dev, _devcon, _cb_vs_vertexshader, _cb_ps_pixelshader);
    _particles.push_back(blackHole2);
    velocity = GetOrbitalVelocity(blackHole1, blackHole2);
    blackHole2->SetVelocity(velocity.x * 0.7, velocity.y * 0.7);

    for (int j = 0; j < N2; ++j)
    {
        float x(0), y(0), r(0);
        r = spawn_range * sqrt((double)rand() / RAND_MAX) + 10.0f;

        float theta = ((double)rand() / RAND_MAX) * 2 * PI;
        x = galaxy_center.x + r * cos(theta);
        y = galaxy_center.y + r * sin(theta);

        Entity* newParticle = new Entity();
        newParticle->Create(particle_radius, particle_mass, _imageShaderResourceView, XMFLOAT3(x, y, 0.0f), XMFLOAT2(0.0f, 0.0f));
        newParticle->Initialize(_dev, _devcon, _cb_vs_vertexshader, _cb_ps_pixelshader);
        _particles.push_back(newParticle);
        velocity = GetOrbitalVelocity(blackHole2, newParticle);
        newParticle->SetVelocity(velocity.x += blackHole2->GetVelocityFloat2().x, velocity.y += blackHole2->GetVelocityFloat2().y);
    }
}