#include "IOCP.h"
#include "SHA1.h"
#pragma comment(lib, "ws2_32.lib")

/////////////////////////////////////////////////////////////////////////////////////////////

CIOCP::CIOCP(string skin)
{
    // m_mcontralcenter.clear();
    m_strskin = skin;
    ::InitializeCriticalSection(&crtc_sec);
}

//构造函数
CIOCP::~CIOCP()
{
    Close1();
}

void dealiocp(LPVOID lp)
{
    CIOCP* io = (CIOCP*)lp;

    if(io->InitAll() == FALSE)
    {
        return;
    }

    io->MainLoop();
}

void CIOCP::Notify(TNotifyUI& msg)
{
    if(msg.sType == _T("windowinit"))
        OnPrepare(msg);
    else if(_tcsicmp(msg.sType, "itemactivate") == 0)
    {
        string pSenderName = msg.pSender->GetName();
        string pClassName = msg.pSender->GetClass();

        if(pClassName == "ListTextElementUI")
        {
            CListTextElementUI* p1 = (CListTextElementUI*)msg.pSender;
            string str = p1->GetText(5);
            gstring::copyToclip(str.c_str(), str.size());
            p1->SetText(5, "");
            //    string val = list1.getCellText(nnn, 4);
            //    gstring::copyToclip(val.c_str(), val.size());
            //    list1.setItemText("", nnn, 4);
            //p1->GetText()
        }

        //MessageBoxA(m_hWnd, 0, 0, 0);
    }
    else if(msg.sType == _T("click"))
    {
        if(msg.pSender->GetName() == "closebtn")
        {
            Shell_NotifyIcon(NIM_DELETE, &nid);
            //SendMessageA(WM_SYSCOMMAND, SC_CLOSE, 0);
            //SendMessageA(WM_SYSCOMMAND, SC_CLOSE, 0);
            Close();
        }
        else if(msg.pSender->GetName() == "minbtn")
        {
            Shell_NotifyIcon(NIM_ADD, &nid); //在托盘区添加图标de函数
            this->ShowWindow(false);
            //SendMessageA(WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }
        else if(msg.pSender->GetName() == "maxbtn")
        {
            SendMessageA(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }
        else if(msg.pSender->GetName() == "closesocket")
        {
            int n = m_plistuser->GetCurSel();

            if(n == -1)
            {
                gstring::tip("请勾选数据");
                return;
            }

            CListTextElementUI* pText = (CListTextElementUI*) m_plistuser->GetItemAt(n);
            string lpio = pText->GetText(2);
            DWORD lp = strtol(lpio.c_str(), NULL, 16);
            IOCP_IO_PTR pIo = (IOCP_IO_PTR)lp;
            closesocket(pIo->socket);
        }
        else if(msg.pSender->GetName() == "gaywaylist")
        {
            int         op, op_len, nRet;
            IOCP_IO_PTR lp_start = NULL;
            IO_POS      pos;
            lp_start =  m_io_group.GetHeadPosition(pos);

            if(lp_start == NULL)
            {
                PostLog("服务端还末开启");
                return;
            }

            //glog::trace("come on CheckForInvalidConnection");
            while(lp_start != NULL)
            {
                op_len = sizeof(op);
                nRet = getsockopt(lp_start->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);
                int len = 0;

                if(op != 0xffffffff)
                {
                    len = op - lp_start->timelen;
                }

                if(lp_start->fromtype == SOCKET_FROM_GAYWAY)
                {
                    PostLog("网关:%s 在线间隔收到的消息:%d秒 通信指针:%p socket:%d", lp_start->gayway, len, lp_start, lp_start->socket);
                }
                else if(lp_start->fromtype == SOCKET_FROM_WEBSOCKET)
                {
                    PostLog("来自网页 在线间隔收到的消息:%d秒 通信指针:%p socket:%d",  len, lp_start, lp_start->socket);
                }
                else
                {
                    PostLog("未知连接 在线间隔收到的消息:%d秒 通信指针:%p socket:%d", len, lp_start, lp_start->socket);
                }

                lp_start = m_io_group.GetNext(pos);
            }

            PostLog("网关总数:%d", m_mcontralcenter.size());
        }
        else if(msg.pSender->GetName() == "weblist")
        {
            string  str = "aaaa";
            _RecordsetPtr rs = dbopen->ExecuteWithResSQL(str.c_str());
        }
        else if(msg.pSender->GetName() == "start")
        {
            DWORD tid = 0;
            HANDLE h1 = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)dealiocp, this, NULL, &tid);
            CloseHandle(h1);
            //PostLog("dddd");
        }
        else if(msg.pSender->GetName() == "clear")
        {
            m_pRishLog->SetText("");
        }
        else if(msg.pSender->GetName() == "senddata")
        {
            int n = m_plistuser->GetCurSel();
            string lpiostr =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 2);
            string lpkeystr =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 3);
            string data =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 4);
            ULONG_PTR io = (ULONG_PTR)strtol(lpiostr.c_str(), NULL, 16);
            ULONG_PTR ik = (ULONG_PTR)strtol(lpiostr.c_str(), NULL, 16);

            if(io == 0 || ik == 0)
            {
                PostLog("请选择列表");
                return;
            }

            SendData(io, ik, data);
        }
        else if(msg.pSender->GetName() == "setdata")
        {
            string data = m_pData->GetText();
            setItemText(m_plistuser, m_plistuser->GetCurSel(), 4, data);
        }
        else if(msg.pSender->GetName() == "deletebtn")
        {
            CControlUI* p1 = (CControlUI*) strtol(m_pData->GetText(), NULL, 16);
            int iIndex = m_plistuser->GetItemIndex(p1);

            if(iIndex != -1)
            {
                m_plistuser->Remove(p1);
            }
        }
        else if(msg.pSender->GetName() == "sendcrc")
        {
            int n = m_plistuser->GetCurSel();
            string lpiostr =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 2);
            string lpkeystr =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 3);
            string data =   m_pData->GetText();;
            IOCP_IO_PTR io = (IOCP_IO_PTR)strtol(lpiostr.c_str(), NULL, 16);
            IOCP_KEY_PTR ik = (IOCP_KEY_PTR)strtol(lpiostr.c_str(), NULL, 16);

            if(io == 0 || ik == 0)
            {
                PostLog("请选择列表");
                return;
            }

            string   vvv = gstring::replace(data, " ", "");
            char* p1 = (char*)vvv.c_str();
            BYTE b2[1024] = {0};
            int i = 0;

            while(*p1 != '\0')
            {
                char data[3] = {0};
                memcpy(data, p1, 2);
                b2[i] = strtol(data, NULL, 16);
                p1 += 2;
                i++;
            }

            SHORT crc16 = usMBCRC16(b2, i);
            b2[i] = crc16 & 0xff;
            b2[i + 1] = crc16 >> 8 & 0xff;
            int sendlen = i + 2;
            InitIoContext(io);
            memcpy(io->buf, b2, sendlen);
            io->wsaBuf.len = sendlen;
            io->wsaBuf.buf = io->buf;
            io->operation = IOCP_WRITE;
            DataAction(io, ik);
        }
        else if(msg.pSender->GetName() == "sendconfig")
        {
            string str = m_cmbworkmode->GetText();
            str.append("\r\n");
            PostLog("发送的指命令:%s", str.c_str());
            int n = m_plistuser->GetCurSel();

            if(n == -1)
            {
                PostLog("请勾选列表");
                return;
            }

            string lpiostr =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 2);
            string lpkeystr =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 3);
            IOCP_IO_PTR io = (IOCP_IO_PTR)strtol(lpiostr.c_str(), NULL, 16);
            IOCP_KEY_PTR ik = (IOCP_KEY_PTR)strtol(lpiostr.c_str(), NULL, 16);
            InitIoContext(io);
            memcpy(io->buf, str.c_str(), str.size());
            io->wsaBuf.len = str.size();
            io->wsaBuf.buf = io->buf;
            io->operation = IOCP_WRITE;
            DataAction(io, ik);
        }
    }
}

//析构函数
/*-------------------------------------------------------------------------------------------
函数功能：关闭并清除资源
函数说明：
函数返回：
-------------------------------------------------------------------------------------------*/
void CIOCP::Close1()
{
    int                 i;
    IO_POS              pos;
    IOCP_IO_PTR lp_io;
    CloseHandle(m_h_iocp);
    m_io_group.GetHeadPosition(pos);

    //  while( pos != NULL )
    //  {
    //      lp_io = m_io_group.GetNext( pos );
    //
    //      closesocket( lp_io->socket );
    //  }

    for(i = 0; i < m_n_thread_count; i++)
    {
        CloseHandle(m_h_thread[i]);
        m_h_thread[i] = NULL;
    }
}

/*-------------------------------------------------------------------------------------------
    函数功能：初始化IO结点
    函数说明：
    函数返回：
    -------------------------------------------------------------------------------------------*/
void CIOCP::InitIoContext(IOCP_IO_PTR lp_io)
{
    memset(&lp_io->ol,  0, sizeof(WSAOVERLAPPED));
    memset(&lp_io->buf, 0, BUFFER_SIZE);
    lp_io->wsaBuf.buf       = lp_io->buf;
    lp_io->wsaBuf.len       = BUFFER_SIZE;
}

/*-------------------------------------------------------------------------------------------
    函数功能：初始化侦听SOCKET端口，并和完成端口连接起来。
    函数说明：
    函数返回：成功，TRUE；失败，FALSE
    -------------------------------------------------------------------------------------------*/
BOOL CIOCP::InitSocket()
{
    m_listen_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    if(INVALID_SOCKET == m_listen_socket)
    {
        glog::traceErrorInfo("call WSASocket", WSAGetLastError());
        return FALSE;
    }

    IOCP_KEY_PTR  lp_key = m_key_group.GetBlank();
    lp_key->socket = m_listen_socket;
    HANDLE hRet = CreateIoCompletionPort((HANDLE)m_listen_socket, m_h_iocp, (DWORD)lp_key, 0);

    if(hRet == NULL)
    {
        closesocket(m_listen_socket);
        m_key_group.RemoveAt(lp_key);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------------------------
    函数功能：关闭所有线程
    函数说明：
    函数返回：
    -------------------------------------------------------------------------------------------*/
void CIOCP::CloseThreadHandle(int count)
{
    if(count <= 0)
    {
        return;
    }

    for(int i = 0; i < count; i++)
    {
        CloseHandle(m_h_thread[i]);
        m_h_thread[i] = INVALID_HANDLE_VALUE;
    }
}

/*-------------------------------------------------------------------------------------------
函数功能：将侦听端口和自己的IP，PORT绑定，并开始侦听
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::BindAndListenSocket()
{
    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(SOCKADDR_IN));
    addr.sin_family         = AF_INET;
    //addr.sin_addr.s_addr    = htons(ADDR);//inet_addr(ADDR);
    //addr.sin_port           = htons(PORT);
    addr.sin_addr.s_addr    = inet_addr(ip); //inet_addr(ip);//htons(ip);//inet_addr(ADDR);
    addr.sin_port           = htons(port);
    int nRet;
    nRet = bind(m_listen_socket, (SOCKADDR*)&addr, sizeof(SOCKADDR));

    if(SOCKET_ERROR == nRet)
    {
        glog::traceErrorInfo("call bind()", WSAGetLastError());
        return FALSE;
    }

    nRet = listen(m_listen_socket, 20);

    if(SOCKET_ERROR == nRet)
    {
        glog::GetInstance()->AddLine("listen fail! error info:%s", getErrorInfo(WSAGetLastError()).c_str());
        glog::traceErrorInfo("call bind()", WSAGetLastError());
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------------------------------
函数功能：根据CPU的数目，启动相应数量的数据处理线程
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::StartThread()
{
    int i;
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    m_n_thread_count = sys_info.dwNumberOfProcessors > MAXTHREAD_COUNT ? MAXTHREAD_COUNT : sys_info.dwNumberOfProcessors;
    //m_n_thread_count = 1;

    for(i = 0; i < m_n_thread_count; i++)
    {
        DWORD tid = 0;
        m_h_thread[i] = CreateThread(NULL, 0, CompletionRoutine, (LPVOID)this, 0, &tid);
        glog::GetInstance()->AddLine("i:%d ThreadId:%d", i, tid);

        if(NULL == m_h_thread[i])
        {
            CloseThreadHandle(i);
            CloseHandle(m_h_iocp);
            return FALSE;
        }

        //PostLog("start %d thread:%d", i + 1, tid);
    }

    return TRUE;
}


/*-------------------------------------------------------------------------------------------
函数功能：发出一定数量的连接
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::PostAcceptEx()
{
    int     count = 2;
    DWORD   dwBytes = 0;
    BOOL    bRet;

    for(int i = 0; i < count; i++)
    {
        SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

        if(INVALID_SOCKET == socket)
        {
            glog::traceErrorInfo("WSASocket", WSAGetLastError());
            continue;
        }

        IOCP_IO_PTR lp_io = m_io_group.GetBlank();
        InitIoContext(lp_io);
        lp_io->socket           = socket;
        lp_io->operation        = IOCP_ACCEPT;
        lp_io->state            = SOCKET_STATE_NOT_CONNECT;
        lp_io->fromtype = SOCKET_FROM_UNKNOW;
        lp_io->ibeathit = 0;
        //lp_io->loginstatus = SOCKET_STATUS_UNKNOW;
        lp_io->lp_key = NULL;
        lp_io->timelen = 0;
        memset(lp_io->day, 0, 20);
        memset(lp_io->gayway, 0, 20);
        //glog::GetInstance()->AddLine("post accecptex socket:%d IOCP_IO_PTR:%p", socket, lp_io);
        /////////////////////////////////////////////////
        bRet = lpAcceptEx(m_listen_socket, lp_io->socket, lp_io->buf,
                          lp_io->wsaBuf.len - 2 * (sizeof(SOCKADDR_IN) + 16),
                          // 0,
                          sizeof(SOCKADDR_IN) + 16,
                          sizeof(SOCKADDR_IN) + 16,
                          &dwBytes, &lp_io->ol);

        if((bRet == FALSE) && (WSA_IO_PENDING != WSAGetLastError()))
        {
            closesocket(socket);
            m_io_group.RemoveAt(lp_io);
            // cout << "post acceptex fail:" << WSAGetLastError() << endl;
            glog::traceErrorInfo("acceptex", WSAGetLastError());
            continue;
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------------------------
函数功能：处理数据函数
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::HandleData(IOCP_IO_PTR lp_io, int nFlags, IOCP_KEY_PTR lp_key, DWORD dwByte)
{
    switch(nFlags)
    {
        case IOCP_COMPLETE_ACCEPT:
            {
                if(SOCKET_STATE_CONNECT != lp_io->state)
                {
                    lp_io->state = SOCKET_STATE_CONNECT;
                }

                char szPeerAddress[50];
                SOCKADDR_IN *addrClient = NULL, *addrLocal = NULL;
                char ip[50] = {0};
                int nClientLen = sizeof(SOCKADDR_IN), nLocalLen = sizeof(SOCKADDR_IN);
                lpGetAcceptExSockaddrs(lp_io->buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&addrLocal, &nLocalLen, (LPSOCKADDR*)&addrClient, &nClientLen);
                char* ip1 = inet_ntoa(addrClient->sin_addr);
                sprintf(szPeerAddress, "%s:%d", ip1, addrClient->sin_port);
                CListTextElementUI* pListElement = new CListTextElementUI;
                m_plistuser->Add(pListElement);
                lp_io->pUserData = pListElement;
                // m_pData->SetText(gstring::int2str((int)pListElement, 16).c_str());
                char vvv[20] = {0};
                int n = m_plistuser->GetCount();
                sprintf(vvv, "%d", n);
                pListElement->SetText(0, vvv);
                pListElement->SetText(1, szPeerAddress);
                sprintf(vvv, "%p", lp_io);
                pListElement->SetText(2, vvv);
                sprintf(vvv, "%p", lp_key);
                pListElement->SetText(3, vvv);
                glog::GetInstance()->AddLine("客户端上线:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
                PostLog("客户端上线:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
                lp_io->operation    = IOCP_WRITE;
                memset(lp_io->buf, 0, BUFFER_SIZE);
                unsigned char hexData[6] =
                {
                    0x61, 0x62, 0x63, 0x64, 0x65, 0x66
                };
                memcpy(lp_io->buf, hexData, sizeof(hexData));
                lp_io->wsaBuf.buf = lp_io->buf;
                lp_io->wsaBuf.len = sizeof(hexData);
                lp_io->operation = IOCP_WRITE;
            }
            break;

        case IOCP_COMPLETE_ACCEPT_READ:
            {
                if(SOCKET_STATE_CONNECT_AND_READ != lp_io->state)
                {
                    lp_io->state = SOCKET_STATE_CONNECT_AND_READ;
                }

                char szPeerAddress[50] = {0};
                char szLocalAddress[50] = {0};
                SOCKADDR_IN *addrClient = NULL;
                SOCKADDR_IN *addrLocal = NULL;
                int nClientLen = sizeof(SOCKADDR_IN);
                int nLocalLen = sizeof(SOCKADDR_IN);
                lpGetAcceptExSockaddrs(lp_io->wsaBuf.buf, lp_io->wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&addrLocal, &nLocalLen, (LPSOCKADDR*)&addrClient, &nClientLen);
                char* ip1 = inet_ntoa(addrClient->sin_addr);
                sprintf(szPeerAddress, "%s:%d", ip1, addrClient->sin_port);
                char* localip = inet_ntoa(addrLocal ->sin_addr);
                SHORT localport = ntohs(addrLocal ->sin_port);
                sprintf(szLocalAddress, "%s:%d", localip, localport);
                glog::trace("本地ip:%s 远程ip:%s", szLocalAddress, szPeerAddress);
                CListTextElementUI* pListElement = new CListTextElementUI;
                m_plistuser->Add(pListElement);
                lp_io->pUserData = pListElement;
                // m_pData->SetText(gstring::int2str((int)pListElement, 16).c_str());
                char vvv[20] = {0};
                int n = m_plistuser->GetCount();
                sprintf(vvv, "%d", n);
                pListElement->SetText(0, vvv);
                pListElement->SetText(1, szPeerAddress);
                sprintf(vvv, "%p", lp_io);
                pListElement->SetText(2, vvv);
                sprintf(vvv, "%p", lp_key);
                pListElement->SetText(3, vvv);
                string data = gstring::char2hex(lp_io->buf, dwByte);
                // glog::GetInstance()->AddLine("包长度:%d 包数据:%s", dwByte, data.c_str());
                pListElement->SetText(5, data.c_str());
                pListElement->SetText(6, lp_io->buf);
                char lenstr[20] = {0};
                sprintf(lenstr, "%d", dwByte);
                pListElement->SetText(7, lenstr);
                glog::GetInstance()->AddLine("客户端上线:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
                PostLog("客户端上线:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
                string req = lp_io->buf;
                string res;
                int wsconn = wsHandshake(req, res);

                if(wsconn == WS_STATUS_CONNECT)
                {
                    InitIoContext(lp_io);
                    //lp_io->operation = IOCP_WRITE;
                    lp_io->fromtype = SOCKET_FROM_WEBSOCKET;
                    pListElement->SetText(8, "web客户端(2)");
                    strcpy(lp_io->gayway, "web客户端(2)");
                    memcpy(lp_io->buf, res.c_str(), res.size());
                    PostLog("web端上线....");
                    lp_io->wsaBuf.len = res.size();
                    lp_io->operation = IOCP_WRITE;
                    break;
                }
                else
                {
                    if(dwByte == 20)
                    {
                        int nlen = dwByte > 20 ? 20 : dwByte;
                        char id[30] = {0};
                        memcpy(id, lp_io->buf, nlen);
                        SHORT s = usMBCRC16((UCHAR*)id, dwByte - 2);
                        BYTE blow = s & 0xff;
                        BYTE bhigh = s >> 8 & 0xff;
                        char crc32[20] = {0};
                        sprintf(crc32, "%02x%02x", blow, bhigh);

                        if(id[dwByte - 2] == blow && id[dwByte - 1] == bhigh)
                        {
                            lp_io->fromtype = SOCKET_FROM_GAYWAY;
                            char addrarea[20] = {0};
                            memcpy(addrarea, lp_io->buf, dwByte - 2);
                            setOnline(addrarea, 1);
                            strcpy(lp_io->gayway, addrarea);
                            pListElement->SetText(8, addrarea);
                            map<string, IOCP_IO_PTR>::iterator it = m_mcontralcenter.find(addrarea);

                            if(it == m_mcontralcenter.end())
                            {
                                m_mcontralcenter.insert(pair<string, IOCP_IO_PTR>(addrarea, lp_io));
                            }
                            else
                            {
                                it->second = lp_io;
                            }

                            unsigned char hexData[8] =
                            {
                                0x01, 0x03, 0x1E, 0xD8, 0x00, 0x02, 0x42, 0x18
                            };
                            memset(lp_io->buf, 0, BUFFER_SIZE);
                            memcpy(lp_io->buf, hexData, sizeof(hexData));
                            lp_io->wsaBuf.buf = lp_io->buf;
                            lp_io->wsaBuf.len = sizeof(hexData);
                            lp_io->operation = IOCP_WRITE;
                        }
                    }
                }
            }
            break;

        case IOCP_COMPLETE_READ:
            {
                ////cout<<"read a data!"<<lp_io->buf<<endl;
                //printf("read a data! socket:%d \n", lp_io->socket);
                //// lp_io->operation    = IOCP_WRITE;
                //memset(&lp_io->ol, 0, sizeof(lp_io->ol));
            }
            break;

        case IOCP_COMPLETE_WRITE:
            {
                //PostLog("write a data!");
                lp_io->operation    = IOCP_READ;
                InitIoContext(lp_io);
            }
            break;

        default:
            {
                glog::trace("handleData do nothing!");
                return FALSE;
            }
    }

    return TRUE;
}

///*-------------------------------------------------------------------------------------------
/*函数功能：发出一些重叠动作
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::DataAction(IOCP_IO_PTR lp_io, IOCP_KEY_PTR lp_key)
{
    DWORD   dwBytes;
    int     nRet;
    DWORD   dwFlags;

    switch(lp_io->operation)
    {
        case IOCP_WRITE:
            {
                nRet = WSASend(lp_io->socket, &lp_io->wsaBuf, 1, &dwBytes, 0, &lp_io->ol, NULL);

                if((nRet == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
                {
                    string errinfo = getErrorInfo(WSAGetLastError());
                    glog::AddLine("DataAction->WSASend Errinfo:%s", errinfo.c_str());
                    closesocket(lp_io->socket);
                    m_io_group.RemoveAt(lp_io);
                    m_key_group.RemoveAt(lp_io->lp_key);
                    return FALSE;
                }
            }
            break;

        case IOCP_READ:
            {
                dwFlags = 0;
                nRet = WSARecv(lp_io->socket, &lp_io->wsaBuf, 1, &dwBytes, &dwFlags, &lp_io->ol, NULL);

                if((nRet == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
                {
                    string errinfo = getErrorInfo(WSAGetLastError());
                    glog::AddLine("DataAction->WSARecv Errinfo:%s", errinfo.c_str());
                    closesocket(lp_io->socket);
                    m_io_group.RemoveAt(lp_io);
                    m_key_group.RemoveAt(lp_key);
                    return FALSE;
                }
            }
            break;

        case IOCP_END:
            {
                ExitSocket(lp_io, lp_key, GetLastError());
                //PostLog(" DataAction->IOCP_END  关闭socket:%p   ", lp_io);
                //closesocket(lp_io->socket);
                //m_io_group.RemoveAt(lp_io);
                //m_key_group.RemoveAt(lp_key);
                //int n = m_io_group.GetCount();
                //int n1 = m_io_group.GetBlankCount();
                //PostLog("IOCP_END lp_io:%p  \nlist1 count:%d list0 count:%d from:%d", lp_io, n, n1, lp_io->fromtype);
            }
            break;

        default:
            {
                PostLog("DataAction do nothing!------------------------------------------");
                return FALSE;
            }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------------------------
函数功能：得到MS封装的SOCKET函数指针，这样可以提高速度
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::GetFunPointer()
{
    DWORD dwRet, nRet;
    nRet = WSAIoctl(m_listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &g_GUIDAcceptEx,
                    sizeof(g_GUIDAcceptEx),
                    &lpAcceptEx,
                    sizeof(lpAcceptEx),
                    &dwRet, NULL, NULL);

    if(SOCKET_ERROR == nRet)
    {
        closesocket(m_listen_socket);
        cout << "get acceptex fail!" << WSAGetLastError() << endl;
        return FALSE;
    }

    nRet = WSAIoctl(
               m_listen_socket,
               SIO_GET_EXTENSION_FUNCTION_POINTER,
               &g_GUIDTransmitFile,
               sizeof(g_GUIDTransmitFile),
               &lpTransmitFile,
               sizeof(lpTransmitFile),
               &dwRet, NULL, NULL);

    if(nRet == SOCKET_ERROR)
    {
        closesocket(m_listen_socket);
        cout << "get transmitfile fail!" << WSAGetLastError() << endl;
        return FALSE;
    }

    nRet = WSAIoctl(m_listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &g_GUIDGetAcceptExSockaddrs,
                    sizeof(g_GUIDGetAcceptExSockaddrs),
                    &lpGetAcceptExSockaddrs,
                    sizeof(lpGetAcceptExSockaddrs),
                    &dwRet, NULL, NULL);

    if(nRet == SOCKET_ERROR)
    {
        closesocket(m_listen_socket);
        cout << "get lpGetAcceptExSockaddrs fail!" << WSAGetLastError() << endl;
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------------------------------
函数功能：注册FD_ACCEPTG事件到m_h_accept_event事件，以便所有发出去的连接耗耗尽时，得到通知。
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::RegAcceptEvent()
{
    int     nRet;
    m_h_accept_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    if(NULL == m_h_accept_event)
    {
        return FALSE;
    }

    nRet = WSAEventSelect(m_listen_socket, m_h_accept_event, FD_ACCEPT);

    if(nRet != 0)
    {
        CloseHandle(m_h_accept_event);
        return FALSE;
    }

    return TRUE;
}
/*-------------------------------------------------------------------------------------------
函数功能：得到连接上来的客户端IP和PORT
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::GetAddrAndPort(char*buf, char ip[], UINT & port)
{
    int     len = BUFFER_SIZE - sizeof(SOCKADDR_IN) - 16;
    char    *lp_buf = buf + len;    //直接读取远端地址
    SOCKADDR_IN addr;
    memcpy(&addr, lp_buf, sizeof(SOCKADDR_IN));
    port    = ntohl(addr.sin_port);
    strcpy(ip, inet_ntoa(addr.sin_addr));
    MSG("客户IP为：");
    MSG(ip);
    MSG("客户端口为：");
    MSG(port);
    return TRUE;
}

void CIOCP::DealWebsockMsg(IOCP_IO_PTR& lp_io, IOCP_KEY_PTR& lp_key, string jsondata, int len)
{
    PostLog("长度:%d web端命令:%s ", len, jsondata.c_str());
    Json::Value root;
    Json::Reader reader;

    if(reader.parse(jsondata.c_str(), root))
    {
        if(root.isObject() == TRUE)
        {
            Json::Value msgType = root["msg"];
            Json::Value isres = root["res"];
            Json::Value tosend = root["data"];

            if(msgType.isString() && tosend.isString() && tosend != "")
            {
                if(msgType == "sensor")
                {
                    string addrarea = root["comaddr"].asString();
                    map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);

                    if(ite != m_mcontralcenter.end())
                    {
                        PostThreadMessageA(ThreadId, WM_USER + 4, (WPARAM)ite->second, (LPARAM)0);
                    }
                }
                else if(msgType == "loop")
                {
                    string addrarea = root["comaddr"].asString();
                    map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);

                    if(ite != m_mcontralcenter.end())
                    {
                        PostThreadMessageA(ThreadId, WM_USER + 3, (WPARAM)ite->second, (LPARAM)0);
                    }
                }
                else   if(msgType == "10" || msgType == "03")
                {
                    string data = tosend.asString();
                    data = gstring::replace(data, " ", "");
                    BYTE bitSend[512] = {0};
                    int len = hex2str(data, bitSend);

                    if(len > 0)
                    {
                        string addrarea = root["comaddr"].asString();
                        map<string, IOCP_IO_PTR>::iterator ite = m_mcontralcenter.find(addrarea);

                        if(ite != m_mcontralcenter.end())
                        {
                            BYTE seq = 0;
                            map<string, list<MSGPACK>>::iterator itmsg = m_MsgPack.find(addrarea);

                            if(itmsg != m_MsgPack.end())
                            {
                                list<MSGPACK>v_msg = itmsg->second;
                                seq = v_msg.begin() == v_msg.end() ? 0 : v_msg.back().seq + 1;
                            }
                            else
                            {
                                seq = 0;
                            }

                            seq = seq > 0xf ? 0 : seq;
                            _MSGPACK msg = {0};
                            msg.lp_io = lp_io;
                            msg.seq = seq;
                            msg.timestamp = time(NULL);
                            msg.root = root;

                            if(itmsg == m_MsgPack.end())
                            {
                                list<_MSGPACK>v_msgpack;
                                m_MsgPack.insert(pair<string, list<MSGPACK>>(addrarea, v_msgpack));
                            }

                            itmsg = m_MsgPack.find(addrarea);
                            itmsg->second.push_back(msg);
                            SHORT crc16 = usMBCRC16(bitSend, len);
                            bitSend[len] = crc16 & 0xff;
                            bitSend[len + 1] = crc16 >> 8 & 0xff;
                            int sendlen = len + 2;
                            string tosenddata = gstring::char2hex((const char*)bitSend, sendlen);
                            PostLog("转换后:%s 长度:%d", tosenddata.c_str(), sendlen);
                            IOCP_IO_PTR lp_io1 = ite->second;
                            memcpy(lp_io1->buf, bitSend, sendlen);
                            lp_io1->wsaBuf.buf = lp_io1->buf;
                            lp_io1->wsaBuf.len = sendlen;
                            lp_io1->operation = IOCP_WRITE;
                            DataAction(lp_io1, lp_io1->lp_key);
                        }
                    }
                }
            }
        }
    }
}
BOOL CIOCP::IsBreakPack(IOCP_IO_PTR & lp_io, BYTE src[], int len)
{
    if(lp_io->fromtype == SOCKET_FROM_GAYWAY)
    {
        SHORT len1 = *(SHORT*)&src[1];
        SHORT len2 = *(SHORT*)&src[3];
        SHORT len3 = len1 >> 2;

        if((len3 > len - 8) && src[0] == 0x68)
        {
            return TRUE;
        }
    }

    return FALSE;
    //if(len < 6)
    //  {
    //    return FALSE;
    //  }
    //if(src[0] == 0x68)
    //  {
    //    int aa = 44;
    //  }
    //SHORT len1 = *(SHORT*)&src[1];
    //SHORT len2 = *(SHORT*)&src[3];
    //if(src[0] == 0x68 && len1 == len2 && src[5] == 0x68)
    //  {
    //    BOOL bAllpack =  checkFlag(src, len);
    //    if(bAllpack == FALSE)
    //      {
    //        return TRUE;
    //      }
    //  }
    return FALSE;
}
BOOL CIOCP::CloseMySocket(IOCP_IO_PTR lp_io)
{
    closesocket(lp_io->socket);
    m_io_group.RemoveAt(lp_io);
    m_key_group.RemoveAt(lp_io->lp_key);
    int n = m_io_group.GetCount();
    int n1 = m_io_group.GetBlankCount();
    glog::trace("\n CloseMySocket  lp_io:%p  list1 count:%d list0 count:%d from:%d", lp_io, n, n1, lp_io->fromtype);
    return TRUE;
}
DWORD CIOCP::TimeThread(LPVOID lp_param)
{
    CIOCP*          lp_this         = (CIOCP*)lp_param;
    MSG msg;

    while(::GetMessage(&msg, NULL, 0, 0))
    {

		//场景号  //回路
        else if(msg.message == WM_USER + 3)
        {
            IOCP_IO_PTR lo = (IOCP_IO_PTR)msg.wParam;

            if(lo)
            {
                BYTE data[8] = {0};
                data[0] = 0x1;
                data[1] = 0x3;
                SHORT infonum = 3929 | 0x1000;               //一次只能10个信息点  10个信息点200字节
                data[2] = infonum >> 8 & 0xff;
                data[3] = infonum & 0xff;
                data[4] = 0;
                data[5] = 3;
                SHORT crc16 = lp_this->usMBCRC16(data, 6);
                data[6] = crc16 & 0x00ff;
                data[7] = crc16 >> 8 & 0x00ff;
                lp_this->InitIoContext(lo);
                memcpy(lo->buf, data, sizeof(data));
                lo->wsaBuf.len = sizeof(data);
                lo->wsaBuf.buf = lo->buf;
                lo->operation = IOCP_WRITE;
                lp_this->DataAction(lo, lo->lp_key);
            }
        }
        //信息点
        else if(msg.message == WM_USER + 4)
        {
            IOCP_IO_PTR lo = (IOCP_IO_PTR)msg.wParam;

            if(lo)
            {
                BYTE data[8] = {0};
                data[0] = 0x1;
                data[1] = 0x3;
                SHORT infonum = 600|0x1000;               //一次只能10个信息点  10个信息点200字节
                data[2] = infonum >> 8 & 0xff;
                data[3] = infonum & 0xff;
                data[4] = 0;
                data[5] = 100;
                SHORT crc16 = lp_this->usMBCRC16(data, 6);
                data[6] = crc16 & 0x00ff;
                data[7] = crc16 >> 8 & 0x00ff;
                lp_this->InitIoContext(lo);
                memcpy(lo->buf, data, sizeof(data));
                lo->wsaBuf.len = sizeof(data);
                lo->wsaBuf.buf = lo->buf;
                lo->operation = IOCP_WRITE;
                lp_this->DataAction(lo, lo->lp_key);
            }
        }
        //采集回路开关
        else if(msg.message == WM_USER + 5)
        {

            IOCP_IO_PTR lo = (IOCP_IO_PTR)msg.wParam;

            if(lo)
            {
                BYTE data[8] = {0};
                data[0] = 0x1;
                data[1] = 0x3;
                SHORT infonum = 700|0x1000;               //一次只能10个信息点  10个信息点200字节
                data[2] = infonum >> 8 & 0xff;
                data[3] = infonum & 0xff;
                data[4] = 0;
                data[5] = 20;
                SHORT crc16 = lp_this->usMBCRC16(data, 6);
                data[6] = crc16 & 0x00ff;
                data[7] = crc16 >> 8 & 0x00ff;
                lp_this->InitIoContext(lo);
                memcpy(lo->buf, data, sizeof(data));
                lo->wsaBuf.len = sizeof(data);
                lo->wsaBuf.buf = lo->buf;
                lo->operation = IOCP_WRITE;
                lp_this->DataAction(lo, lo->lp_key);
            }
        }
    }

    return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*-------------------------------------------------------------------------------------------
函数功能：初始化完成端口及相关的所有东西，并发出每一个10个连接.
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::InitAll()
{
    CoInitialize(NULL);
	
    WSAData data;

    if(WSAStartup(MAKEWORD(2, 2), &data) != 0)
    {
        cout << "WSAStartup fail!" << WSAGetLastError() << endl;
        return FALSE;
    }

    m_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

    if(NULL == m_h_iocp)
    {
        PostLog("CreateIoCompletionPort() failed: errcode:%d", GetLastError());
        return FALSE;
    }

    if(!StartThread())
    {
        PostLog("start thread fail! errcode:%d", GetLastError());
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        CloseHandle(m_h_iocp);
        return FALSE;
    }

    //定时采集线程
    //DWORD tid = 0;
    HANDLE hTreadTime = CreateThread(NULL, NULL, TimeThread, (LPVOID)this, NULL, &ThreadId);
    CloseHandle(hTreadTime);
    ////定时采集线程
    //DWORD tidEmail = 0;
    //HANDLE hTreadEmail = CreateThread(NULL, NULL, TimeEmail, (LPVOID)this, NULL, &tidEmail);
    //CloseHandle(hTreadEmail);

    if(!InitSocket())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        PostLog("Init sociket fail! errcode:%d", GetLastError());
        CloseHandle(m_h_iocp);
        return FALSE;
    }

    if(!BindAndListenSocket())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        PostLog("BindAndListenSocket sociket fail! errcode:%d", GetLastError());
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    if(!GetFunPointer())
    {
        PostLog("GetFunPointer sociket fail! errcode:%d", GetLastError());
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    if(!PostAcceptEx())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        PostLog("PostAcceptEx sociket fail! errcode:%d", GetLastError());
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    if(!RegAcceptEvent())
    {
        PostQueuedCompletionStatus(m_h_iocp, 0, NULL, NULL);
        PostLog("RegAcceptEvent sociket fail! errcode:%d", GetLastError());
        CloseHandle(m_h_iocp);
        closesocket(m_listen_socket);
        return FALSE;
    }

    return TRUE;
}
/*-------------------------------------------------------------------------------------------
函数功能：主循环
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::MainLoop()
{
    DWORD   dwRet;
    int     nCount = 0;
    PostLog("服务端启动...");
    //cout << "Server is running.........." << nCount++ << " times" << endl;
    int ii = 0;

    while(TRUE)
    {
        dwRet = WaitForSingleObject(m_h_accept_event, 60000);

        switch(dwRet)
        {
            case WAIT_FAILED:
                {
                    PostQueuedCompletionStatus(m_h_iocp, 0, 0, NULL);
                    return FALSE;
                }
                break;

            case WAIT_TIMEOUT:
                {
                    //检测集中器超时处理
                    //cout << "Server is running.........." << nCount++ << " times" << endl;
                    CheckForInvalidConnection();
                }
                break;

            case WAIT_OBJECT_0:   //接收到了所有发出的连接都用光了的消息，再次发出连接
                {
                    if(!PostAcceptEx())
                    {
                        PostQueuedCompletionStatus(m_h_iocp, 0, 0, NULL);
                        return FALSE;
                    }
                }
                break;
        }
    }

    return TRUE;
}
/*-------------------------------------------------------------------------------------------
函数功能：看看是否有连接了，但很长时间没有数据的“无效连接”，有的话，就踢掉
函数说明：
函数返回：成功，TRUE；失败，FALSE
-------------------------------------------------------------------------------------------*/
void CIOCP::CheckForInvalidConnection()
{
    int         op, op_len, nRet;
    IOCP_IO_PTR lp_start = NULL;
    IO_POS      pos;
    lp_start =  m_io_group.GetHeadPosition(pos);
    //IOCP_IO_PTR lp_end =       m_io_group.GetEndPosition(pos);
    map<IOCP_IO_PTR, DWORD> m_io;
    m_io.clear();
    map<IOCP_IO_PTR, int> m_mio;
    m_mio.clear();

    //glog::trace("come on CheckForInvalidConnection");
    while(lp_start != NULL)
    {
        if(IsBadReadPtr(lp_start, 4) != 0)
        {
            break;
        }

        op_len = sizeof(op);
        nRet = getsockopt(lp_start->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);

        if(SOCKET_ERROR == nRet)
        {
            glog::traceErrorInfo("CheckForInvalidConnection getsockopt", WSAGetLastError());
            closesocket(lp_start->socket);
            lp_start = m_io_group.GetNext(pos);
            continue;
        }

        if(lp_start->fromtype == SOCKET_FROM_GAYWAY)
        {
            if(op != 0xffffffff)
            {
                int len = op - lp_start->timelen;

                if(len / 60 >= 2)
                {
                    glog::GetInstance()->AddLine("通信指针:%p 网关:%s 超时%d秒 主动关闭 容器长度:%d", lp_start, lp_start->gayway, len, m_io_group.GetCount());
                    EnterCriticalSection(&crtc_sec);
                    map<string, IOCP_IO_PTR>::iterator  it =  m_mcontralcenter.find(lp_start->gayway);

                    if(it == m_mcontralcenter.end())
                    {
                        setOnline(lp_start->gayway, 0);
                    }
                    else
                    {
                        if(it->second == lp_start)
                        {
                            setOnline(lp_start->gayway, 0);
                            m_mcontralcenter.erase(it);
                        }
                    }

                    LeaveCriticalSection(&crtc_sec);
                    closesocket(lp_start->socket);
                    break;
                }
            }
        }

        if(lp_start->state == SOCKET_STATE_CONNECT || lp_start->state == SOCKET_STATE_CONNECT_AND_READ)
        {
            if(lp_start->fromtype == SOCKET_FROM_WEBSOCKET)
            {
                int len = op - lp_start->timelen;

                if(len / 60 >= 20)
                {
                    glog::GetInstance()->AddLine("主动关闭网页客户端");
                    closesocket(lp_start->socket);
                    break;
                }

                //网页20分钟主动干掉
            }
            else if(lp_start->fromtype == SOCKET_FROM_UNKNOW)
            {
                int len = op - lp_start->timelen;

                if(len / 60 >= 5)
                {
                    glog::GetInstance()->AddLine("主动关闭末知客户端");
                    closesocket(lp_start->socket);
                    break;
                }
            }
        }

        lp_start = m_io_group.GetNext(pos);
    }
}
//
/*-------------------------------------------------------------------------------------------
函数功能：数据处理线程函数
函数说明：
函数返回：
-------------------------------------------------------------------------------------------*/
DWORD CIOCP::CompletionRoutine(LPVOID lp_param)
{
    CIOCP*          lp_this         = (CIOCP*)lp_param;
    int             nRet;
    BOOL            bRet;
    DWORD           dwBytes         = 0;
    HANDLE          hRet;
    IOCP_KEY_PTR    lp_key          = NULL;
    IOCP_IO_PTR     lp_io           = NULL;
    LPWSAOVERLAPPED lp_ov           = NULL;
    IOCP_KEY_PTR    lp_new_key      = NULL;

    while(TRUE)
    {
        bRet = GetQueuedCompletionStatus(lp_this->m_h_iocp, &dwBytes, (LPDWORD)&lp_key, &lp_ov, INFINITE);  //
        lp_io   = (IOCP_IO_PTR)lp_ov;

        //退出处理
        if(dwBytes == 0 && lp_io->operation != IOCP_ACCEPT)
        {
            lp_this->ExitSocket(lp_io, lp_key, WSAGetLastError());
            glog::GetInstance()->AddLine("退出 ErrorInfo:%s", lp_this->getErrorInfo(WSAGetLastError()).c_str());
            continue;
        }

        //socket 通信时长
        int op_len = 0;
        int op = 0;
        op_len = sizeof(op);
        nRet = getsockopt(lp_io->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);

        if(op != 0xffffffff)
        {
            lp_io->timelen = op;
            //glog::traceErrorInfo("getsockopt",WSAGetLastError());
            //glog::trace("\nlp_io:%p timelen:%d",lp_io,lp_io->timelen);
        }

        if(bRet && lp_io && lp_key)
        {
            string hexdata = gstring::char2hex(lp_io->buf, dwBytes);
            glog::GetInstance()->AddLine("指针:%p operation:%d 包长度:%d 包数据:%s", lp_io, lp_io->operation, dwBytes, hexdata.c_str());

            switch(lp_io->operation)
            {
                case IOCP_ACCEPT:
                    {
                        nRet = setsockopt(lp_io->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&lp_this->m_listen_socket, sizeof(lp_this->m_listen_socket));

                        if(SOCKET_ERROR == nRet)
                        {
                            glog::GetInstance()->AddLine("CompletionRoutine->IOCP_ACCEPT setsockopt ErroCode:%d", WSAGetLastError());
                            closesocket(lp_io->socket);
                            lp_this->m_io_group.RemoveAt(lp_io);
                            glog::traceErrorInfo("setsockopt", WSAGetLastError());
                            continue;
                        }

                        lp_new_key = lp_this->m_key_group.GetBlank();

                        if(lp_new_key == NULL)
                        {
                            glog::GetInstance()->AddLine("CompletionRoutine->IOCP_ACCEPT m_key_group.GetBlank ErroCode:%d", WSAGetLastError());
                            glog::traceErrorInfo("GetBlank：", WSAGetLastError());
                            closesocket(lp_io->socket);
                            lp_this->m_io_group.RemoveAt(lp_io);
                            continue;
                        }

                        lp_new_key->socket = lp_io->socket;
                        lp_io->lp_key = lp_new_key;
                        //将新建立的SOCKET同完成端口关联起来。
                        hRet = CreateIoCompletionPort((HANDLE)lp_io->socket, lp_this->m_h_iocp, (DWORD)lp_new_key, 0);

                        if(NULL == hRet)
                        {
                            glog::GetInstance()->AddLine("CompletionRoutine->IOCP_ACCEPT CreateIoCompletionPort ErroCode:%d", WSAGetLastError());
                            glog::traceErrorInfo("CreateIoCompletionPort", WSAGetLastError());
                            closesocket(lp_io->socket);
                            lp_this->m_key_group.RemoveAt(lp_new_key);
                            lp_this->m_io_group.RemoveAt(lp_io);
                            continue;
                        }

                        //处理读取到的数据
                        if(dwBytes > 0)
                        {
                            lp_this->HandleData(lp_io, IOCP_COMPLETE_ACCEPT_READ, lp_new_key, dwBytes);
                            bRet = lp_this->DataAction(lp_io, lp_new_key);
                        }
                        else
                        {
                            lp_this->HandleData(lp_io, IOCP_COMPLETE_ACCEPT, lp_new_key, dwBytes);
                            bRet = lp_this->DataAction(lp_io, lp_new_key);
                        }
                    }
                    break;

                case IOCP_READ:
                    {
                        EnterCriticalSection(&lp_this->crtc_sec);
                        lp_this->dealRead(lp_io, lp_key, dwBytes);
ToMsg:
                        bRet = lp_this->DataAction(lp_io, lp_new_key);
                        LeaveCriticalSection(&lp_this->crtc_sec);
                    }
                    break;

                case IOCP_WRITE:
                    {
                        lp_this->HandleData(lp_io, IOCP_COMPLETE_WRITE, lp_new_key, dwBytes);
                        bRet = lp_this->DataAction(lp_io, lp_new_key);
                    }
                    break;

                default:
                    break;
            }
        }
    }

    return 0;
}
BOOL CIOCP::SendData(ULONG_PTR s, ULONG_PTR key, string vvv)
{
    IOCP_IO_PTR  piocp_prt = (IOCP_IO_PTR)s;
    IOCP_KEY_PTR piocp_key = (IOCP_KEY_PTR)key;
    //int n1 = m_listctr->getSelectIndex();
    //string vvv = m_listctr->getCellText(n1, 2);
    vvv = gstring::replace(vvv, " ", "");
    char* p1 = (char*)vvv.c_str();
    BYTE b2[1024] = {0};
    int i = 0;

    while(*p1 != '\0')
    {
        char data[3] = {0};
        memcpy(data, p1, 2);
        b2[i] = strtol(data, NULL, 16);
        p1 += 2;
        i++;
    }

    //string tosend = gstring::hex2char(vvv.c_str(),strlen(vvv.c_str()));
//   strcpy(piocp_prt->buf, tosend.c_str());
    InitIoContext(piocp_prt);
    memcpy(piocp_prt->buf, b2, i);
    piocp_prt->wsaBuf.len = strlen(vvv.c_str()) / 2;
    piocp_prt->wsaBuf.buf = piocp_prt->buf;
    piocp_prt->operation = IOCP_WRITE;
    DataAction(piocp_prt, piocp_prt->lp_key);
    // piocp_prt->operation = IOCP_READ;
    return TRUE;
}
BOOL CIOCP::SendWebsocket(ULONG_PTR s)
{
    IOCP_IO_PTR  lp_io = (IOCP_IO_PTR)s;
    string str = "bbb";
    Json::Value root;
    Json::Value item;
    item["msg"] = "abcdefg";
    root.append(item);
    string srcmsg = root.toStyledString();
    char outmsg[1024] = {};
    this->InitIoContext(lp_io);
    int len = 0;
    this->wsEncodeFrame(srcmsg.c_str(), outmsg, WS_TEXT_FRAME, len);
    lp_io->wsaBuf.len = len;
    memcpy(lp_io->buf, outmsg, len);
    lp_io->wsaBuf.buf = lp_io->buf;
    lp_io->operation = IOCP_WRITE;
    DataAction(lp_io, lp_io->lp_key);
    return TRUE;
}
int CIOCP::hex2str(string str, BYTE tosend[])
{
    string vvv = gstring::replace(str, " ", "");
    char* p1 = (char*)vvv.c_str();
    //BYTE b2[1024] = {0};
    int i = 0;

    while(*p1 != '\0')
    {
        char data[3] = {0};
        memcpy(data, p1, 2);
        tosend[i] = strtol(data, NULL, 16);
        p1 += 2;
        i++;
    }

    //memcpy(tosend,, i);
    return i;
}
int CIOCP::wsHandshake(string & request, string & response)
{
    // 解析http请求头信息
    int ret = WS_STATUS_UNCONNECT;
    std::istringstream stream(request.c_str());
    std::string reqType;
    std::getline(stream, reqType);

    if(reqType.substr(0, 4) != "GET ")
    {
        return ret;
    }

    std::string header;
    std::string::size_type pos = 0;
    std::string websocketKey;

    while(std::getline(stream, header) && header != "\r")
    {
        header.erase(header.end() - 1);
        pos = header.find(": ", 0);

        if(pos != std::string::npos)
        {
            std::string key = header.substr(0, pos);
            std::string value = header.substr(pos + 2);

            if(key == "Sec-WebSocket-Key")
            {
                ret = WS_STATUS_CONNECT;
                websocketKey = value;
                break;
            }
        }
    }

    if(ret != WS_STATUS_CONNECT)
    {
        return ret;
    }

    // 填充http响应头信息
    response = "HTTP/1.1 101 Switching Protocols\r\n";
    response += "Upgrade: websocket\r\n";
    response += "Connection: upgrade\r\n";
    response += "Sec-WebSocket-Accept: ";
    const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    std::string serverKey = websocketKey + magicKey;
    char shaHash[32];
    memset(shaHash, 0, sizeof(shaHash));
    CSHA1 sha1;
    sha1.Update((unsigned char*)serverKey.c_str(), serverKey.size());
    sha1.Final();
    sha1.GetHash((unsigned char *)shaHash);
    serverKey = gstring::base64_encode((unsigned char*)shaHash, strlen(shaHash)) + "\r\n\r\n";
    string strtmp(serverKey.c_str());
    response += strtmp;
    return ret;
}
int CIOCP::wsDecodeFrame(char inFrame[], string & outMessage, int len, BOOL & bBreakPack)
{
    int ret = WS_OPENING_FRAME;
    const char *msg = inFrame;
    const int frameLength = len;

    if(frameLength < 2)
    {
        ret = WS_ERROR_FRAME;
        return ret;
    }

    BYTE Fin =  msg[0] >> 7 & 1;
    BYTE RSV1 = msg[0] >> 6 & 0x01;
    BYTE RSV2 = msg[0] >> 5 & 0x01;
    BYTE RSV3 = msg[0] >> 4 & 0x01;
    BYTE Mask =  msg[1] >> 7 & 0x01;

    //FIN:1位，用于描述消息是否结束，如果为1则该消息为消息尾部,如果为零则还有后续数据包;
    if(Fin != 1)
    {
        ret = WS_ERROR_FRAME;
        return ret;
    }

    // 检查扩展位并忽略
    if(RSV1 == 1 || RSV2 == 1 || RSV3 == 1)
    {
        ret = WS_ERROR_FRAME;
        return ret;
    }

    // mask位, 为1表示数据被加密
    if(Mask != 1)
    {
        ret = WS_ERROR_FRAME;
        return ret;
    }

    BYTE opcode = msg[0] & 0x0f;
    // 操作码
    uint16_t payloadLength = 0;
    uint8_t payloadFieldExtraBytes = 0;

    if(opcode == WS_TEXT_FRAME)
    {
        // 处理utf-8编码的文本帧
        payloadLength = static_cast<uint16_t >(msg[1] & 0x7f);

        if(payloadLength == 0x7e)   //0111 1110     //126 7e  后面两字节是长度 :  127  7f 后面四字节是长度
        {
            uint16_t payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &msg[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }
        else if(payloadLength == 0x7f)
        {
            // 数据过长,暂不支持
            uint32_t payloadLength32b = 0;
            payloadFieldExtraBytes = 4;
            memcpy(&payloadLength32b, &msg[2], payloadFieldExtraBytes);
            payloadLength = ntohl(payloadLength32b);
            //ret = WS_ERROR_FRAME;
        }
    }
    else if(opcode == WS_BINARY_FRAME || opcode == WS_PING_FRAME || opcode == WS_PONG_FRAME)
    {
        // 二进制/ping/pong帧暂不处理
    }
    else if(opcode == WS_CLOSING_FRAME)
    {
        ret = WS_CLOSING_FRAME;
    }
    else
    {
        ret = WS_ERROR_FRAME;
    }

    // 数据解码
    if((ret != WS_ERROR_FRAME) && (payloadLength > 0))
    {
        if(payloadLength > (len - 2 - payloadFieldExtraBytes - 4))
        {
            bBreakPack = TRUE;
            return ret;
        }

        // header: 2字节, masking key: 4字节
        const char *maskingKey = &msg[2 + payloadFieldExtraBytes];
        char *payloadData = new char[payloadLength + 1];
        memset(payloadData, 0, payloadLength + 1);
        memcpy(payloadData, &msg[2 + payloadFieldExtraBytes + 4], payloadLength);

        for(int i = 0; i < payloadLength; i++)
        {
            payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
        }

        outMessage = payloadData;
    }

    return ret;
}
int CIOCP::wsEncodeFrame(string inMessage, char outFrame[], enum WS_FrameType frameType, int& lenret)
{
    int ret = WS_EMPTY_FRAME;
    const uint32_t messageLength = inMessage.size();

    if(messageLength > 32767)
    {
        // 暂不支持这么长的数据
        return WS_ERROR_FRAME;
    }

    uint16_t payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
    // header: 2字节, mask位设置为0(不加密), 则后面的masking key无须填写, 省略4字节
    uint8_t frameHeaderSize = 2 + payloadFieldExtraBytes;
    uint8_t *frameHeader = new uint8_t[frameHeaderSize];
    memset(frameHeader, 0, frameHeaderSize);
    // fin位为1, 扩展位为0, 操作位为frameType
    frameHeader[0] = static_cast<uint8_t>(0x80 | frameType);

    // 填充数据长度
    if(messageLength <= 0x7d)   //125->7d
    {
        frameHeader[1] = static_cast<uint8_t>(messageLength);
    }
    else if(messageLength < 65535)
    {
        frameHeader[1] = 0x7e;
        uint16_t len = htons(messageLength);
        memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);
    }
    else
    {
    }

    // 填充数据
    uint32_t frameSize = frameHeaderSize + messageLength;
    char *frame = new char[frameSize + 1];
    memcpy(frame, frameHeader, frameHeaderSize);
    memcpy(frame + frameHeaderSize, inMessage.c_str(), messageLength);
    frame[frameSize] = '\0';
    memcpy(outFrame, frame, frameSize);
    lenret = frameSize;
    //outFrame = frame;
    delete[] frame;
    delete[] frameHeader;
    return ret;
}

BOOL CIOCP::checkFlag(BYTE vv[], int len)
{
    //if(len < 6)
    //{
    //    return FALSE;
    //}
    //if(vv[0] == 0x68 && vv[5] == 0x68 && vv[len - 1] == 0x16)
    //{
    //    SHORT len1 = *(SHORT*)&vv[1];
    //    SHORT len2 = *(SHORT*)&vv[3];
    //    SHORT len11 = len1 >> 2;
    //    SHORT len22 = len2 >> 2;
    //    if(len11 == len22 && len22 == len - 8)
    //    {
    //        return TRUE;
    //    }
    //}
    return FALSE;
}

BOOL CIOCP::AppendByte(BYTE src[], int& len, pBREAKPCK pack, IOCP_IO_PTR & lp_io)
{
    if(pack != NULL && pack->len > 0)
    {
        int lenall = len + pack->len;

        if(lenall <= BUFFER_SIZE)
        {
            BYTE* allbyte = new BYTE[lenall];
            memset(allbyte, 0, lenall);
            memcpy(allbyte, pack->b, pack->len);
            memcpy(allbyte +  pack->len, src, len);
            delete pack->b;
            pack->b = allbyte;
            pack->len = lenall;
            len = lenall;
            memset(lp_io->wsaBuf.buf, 0, BUFFER_SIZE);
            memcpy(lp_io->wsaBuf.buf, allbyte, lenall);
            lp_io->wsaBuf.len = lenall;
        }
        else
        {
            delete pack->b;
            len = lenall;
        }
    }
    else
    {
        glog::GetInstance()->AddLine("断包原数据空");
    }

    return TRUE;
}
/*
*  comaddr 网关地址
*  C       控制域
*  AFN  功能码
*  SEQ  序列域
*  DA
*  DT
*  v_B  额外数据
* des  接收到的数据
*/
int CIOCP::buidByte(string comaddr, BYTE C, BYTE AFN, BYTE SEQ, SHORT DA, SHORT DT, vector<BYTE>&v_b, BYTE des[])
{
    BYTE addrArea[4] = {0};

    if(comaddr.size() != 8)
    {
        PostLog("通信地址不合格式");
        return 0;
    }

    int n =  hex2str(comaddr, addrArea);

    if(n > 0)
    {
        BYTE hexData[256] = {0};
        hexData[0] = 0x68;
        hexData[5] = 0x68;
        hexData[6] = C;//0x4;   //控制域  启动或是从动  上行或是下行 0x4  0000 0100
        hexData[7] = addrArea[1]; //parseInt(sprintf("0x%02d", addrArea[1]), 16)             //地址域
        hexData[8] = addrArea[0];              //parseInt(sprintf("0x%02d", addrArea[0]), 16)   //地址域
        hexData[9] = addrArea[3];                           //parseInt(sprintf("0x%02d", addrArea[3]), 16)
        hexData[10] = addrArea[2];         //parseInt(sprintf("0x%02d", addrArea[2]), 16)
        hexData[11] = 0x02;  //地址C  单地址或组地址
        hexData[12] = AFN;  //功能码
        hexData[13] = SEQ;  //帧序列
        hexData[15] = DA >> 8 & 0x00ff; //   DA1
        hexData[14] = DA & 0x00ff;
        hexData[17] = DT >> 8 & 0x00ff;
        hexData[16] = DT & 0x00ff;

        for(int i = 0; i < v_b.size(); i++)
        {
            hexData[18 + i] = v_b[i];
        }

        int len1 = 18 - 6 + v_b.size();   //18是固定长度  6报文头
        int len2 = len1 << 2 | 2;
        int  a = len2 >> 8 & 0x000F;
        int b = len2 & 0x00ff;
        hexData[1] = b;               //len1 << 2 | 2;
        hexData[2] = a;
        hexData[3] = b;         //len1 << 2 | 2;
        hexData[4] = a;
        int v1 = 0;
        int len3 = 18 + v_b.size();

        for(int i = 6; i < len3; i++)
        {
            v1 = v1 + hexData[i];
        }

        int jyw = 18 + v_b.size();
        hexData[len3] = v1 % 256;
        hexData[len3 + 1] = 0x16;
        memcpy(des, hexData, len3 + 2);
        //string aaa=   gstring::char2hex((const char*)hexData,len3+2);
        //glog::GetInstance()->AddLine("%s",aaa.c_str());
        return len3 + 2;
    }
}
void CIOCP::ExitSocket(IOCP_IO_PTR & lp_io, IOCP_KEY_PTR & lp_key, int errcode)
{
    EnterCriticalSection(&crtc_sec);
    string towrite = "";
    PostLog("有客户端下线 通信指针:%p  客户端类型:%d", lp_io, lp_io->fromtype);
    //if(errcode != 1236)
    //  {
    //    lp_io->operation = IOCP_DEFAULT;
    //    closesocket(lp_io->socket);
    //  }
    closesocket(lp_io->socket);
    m_io_group.RemoveAt(lp_io);
    m_key_group.RemoveAt(lp_key);
    int n11 = m_io_group.GetCount();
    int n00 = m_io_group.GetBlankCount();
    PostLog("ExitSocket  lp_io:%p  List1 count:%d List2 count:%d 客户端类型:%d", lp_io, n11, n00, lp_io->fromtype);
    //EnterCriticalSection(&crtc_sec);
    //if(lp_io->fromtype == SOCKET_FROM_GAYWAY)
    //  {
    //    //集中器客户端下线
    //    string comaddr = lp_io->gayway;
    //    setOnline(comaddr, 0);
    //    //移除集中器
    //    map<string, IOCP_IO_PTR>::iterator  it = m_mcontralcenter.find(comaddr);
    //    if(it != m_mcontralcenter.end())
    //      {
    //        m_mcontralcenter.erase(it);
    //      }
    //    map<string, list<MSGPACK>>::iterator itMsg = m_MsgPack.find(comaddr);
    //    if(itMsg != m_MsgPack.end())
    //      {
    //        itMsg->second.clear();
    //        m_MsgPack.erase(itMsg);
    //      }
    //  }
    //if(lp_io->fromtype == SOCKET_FROM_WEBSOCKET)
    //  {
    //    map<string, list<MSGPACK>>::iterator it = m_MsgPack.begin();
    //    //  EnterCriticalSection(&crtc_sec);
    //    while(it != m_MsgPack.end())
    //      {
    //        for(auto iter = it->second.begin(); iter != it->second.end();)
    //          {
    //            MSGPACK msg = *iter;
    //            if(msg.lp_io == lp_io)
    //              {
    //                it->second.erase(iter++);
    //                continue;
    //                //break;
    //              }
    //            iter++;
    //          }
    //        it++;
    //      }
    //    //  LeaveCriticalSection(&crtc_sec);
    //  }
    //消息队列删除   消息队列存的是网页客户端
    //  EnterCriticalSection(&crtc_sec);
    DeleteByIo((ULONG_PTR)lp_io->pUserData);

    if(lp_io->fromtype == SOCKET_FROM_GAYWAY)
    {
        string gayway = lp_io->gayway;
        map<string, IOCP_IO_PTR>::iterator  it =  m_mcontralcenter.find(gayway);

        if(it != m_mcontralcenter.end())
        {
            if(it->second == lp_io)
            {
                setOnline(lp_io->gayway, 0);
            }
        }
        else
        {
        }
    }

    lp_io->pUserData = NULL;
    LeaveCriticalSection(&crtc_sec);
}
DWORD WINAPI CIOCP::TimeEmail(LPVOID lp_param)
{
    while(TRUE)
    {
        CIOCP* pThis = (CIOCP*)lp_param;
        pThis->objeamil.SendVecotrEmail();
        Sleep(5000);
    }

    return 1;
}
BOOL CIOCP::dealRead(IOCP_IO_PTR & lp_io, IOCP_KEY_PTR & lp_key, DWORD dwBytes)
{
    if(SOCKET_STATE_CONNECT_AND_READ != lp_io->state)
    {
        lp_io->state = SOCKET_STATE_CONNECT_AND_READ;
    }

    string towrite = "";
    int datalen = dwBytes;
    BYTE* src = (BYTE*)lp_io->buf;
    string data = gstring::char2hex(lp_io->buf, dwBytes);
    towrite = data;
    CListTextElementUI* pElement = (CListTextElementUI*)lp_io->pUserData;

    if(pElement)
    {
        pElement->SetText(8, lp_io->gayway);
        pElement->SetText(5, data.c_str());
        pElement->SetText(6, lp_io->buf);
        char lenstr[20] = {0};
        sprintf(lenstr, "%d", datalen);
        pElement->SetText(6, lp_io->buf);
        pElement->SetText(7, lenstr);
    }

    if(dwBytes > 5)
    {
        if(src[0] == 0x40 && src[1] == 0x44 && src[2] == 0x54 && src[3] == 0x55)
        {
            char info[512] = {0};
            int nn = dwBytes > 512 ? 512 : dwBytes;
            memcpy(info, src, nn);
            PostLog(info);
        }
    }

    if(lp_io->fromtype == SOCKET_FROM_GAYWAY)
    {
        if(dwBytes > 2)
        {
            char pgayway[30] = {0};
            strcpy(pgayway, lp_io->gayway);
            int nn = 0;

            if(strncmp((char*)src, pgayway, strlen(pgayway)) == 0)
            {
                nn = strlen(pgayway) + 2;
                BYTE* psrc = src + nn;
                BYTE* psrc1 = src + nn;
                int packlen = dwBytes - nn;

                //命令
                if(psrc[0] == 0x40 && psrc[1] == 0x44 && psrc[2] == 0x54 && psrc[3] == 0x55)
                {
                    char info[512] = {0};
                    int nn = packlen > 512 ? 512 : packlen;
                    memcpy(info, psrc, nn);
                    PostLog(info);
                    return TRUE;
                }

                if(strncmp((char*)psrc, pgayway, strlen(pgayway)) == 0)
                {
                    lp_io->ibeathit += 1;
                    PostLog("网关[%s] 心跳包 %d ", lp_io->gayway, lp_io->ibeathit);

                    if(lp_io->ibeathit % 5 == 0)
                    {
                        PostLog("网关:%s 5分钟采集传感器", lp_io->gayway);
                        PostThreadMessageA(ThreadId, WM_USER + 4, (WPARAM)lp_io, (LPARAM)0);
                    }

                    if(lp_io->ibeathit % 4 == 0)
                    {
                        PostLog("网关:%s 4分钟采集回路", lp_io->gayway);
                        PostThreadMessageA(ThreadId, WM_USER + 3, (WPARAM)lp_io, (LPARAM)0);
                    }

                    //PostThreadMessageA(ThreadId, WM_USER + 3, (WPARAM)lp_io, (LPARAM)0);
                    return TRUE;
                }
                else
                {
                    if(dwBytes == nn)
                    {
                        lp_io->ibeathit += 1;
                        PostLog("网关[%s] 心跳包 %d ", lp_io->gayway, lp_io->ibeathit);

                        if(lp_io->ibeathit % 5 == 0)
                        {
                            PostLog("网关:%s 5分钟采集信息点", lp_io->gayway);
                            PostThreadMessageA(ThreadId, WM_USER + 4, (WPARAM)lp_io, (LPARAM)0);
                        }

						if(lp_io->ibeathit % 4 == 0)
						{
							PostLog("网关:%s 4分钟采集控制点", lp_io->gayway);
							PostThreadMessageA(ThreadId, WM_USER + 3, (WPARAM)lp_io, (LPARAM)0);
						}

                        //PostThreadMessageA(ThreadId, WM_USER + 3, (WPARAM)lp_io, (LPARAM)0);
                        return TRUE;
                    }

                    if(psrc[0] == 0x1 && psrc[1] == 3)
                    {
                        BYTE  n = 0;
                        int z = 0;
                        time_t tmtamp;
                        struct tm *tm1 = NULL;
                        time(&tmtamp) ;
                        tm1 = localtime(&tmtamp) ;
                        char time2[40] = {0};
                        sprintf(time2, "%04d-%02d-%02d %02d:%02d:%02d", tm1->tm_year + 1900, tm1->tm_mon + 1, tm1->tm_mday, tm1->tm_hour, tm1->tm_min, tm1->tm_sec);

                        while(n < dwBytes - nn)
                        {
                            BYTE len = psrc[n + 2];
                            BYTE endlen = len + 2 + 1 + 2;
                            SHORT crc16 = usMBCRC16(&psrc[n], endlen - 2);
                            SHORT crc16_ = *(SHORT*)(&psrc[n + endlen - 2]);

                            if(crc16 == crc16_)
                            {
                                string data1 = gstring::char2hex((const char*)&psrc[n], endlen);
                                PostLog("网关[%s] 长度:%d 数据:%s", lp_io->gayway, endlen, data1.c_str());
                                    //信息点
                                    if(psrc[2] == packlen - 5 && packlen - 5 == 200)
                                    {
                                        PostLog("采集信息点数据");
                                        int zz = 3;
                                        string gayway = lp_io->gayway;
                                        string sql = "select * from t_sensor where deplayment=1 and l_comaddr=\'";
                                        sql.append(gayway);
                                        sql.append("\' ORDER BY infonum DESC");
                                        _RecordsetPtr rs = dbopen->ExecuteWithResSQL(sql.c_str());
                                        while(rs && !rs->adoEOF)
                                        {
                                            variant_t vinfoval = rs->GetCollect("infonum");
                                            int iinfo = vinfoval;
                                            variant_t vsitenum = rs->GetCollect("sitenum");
                                            int isitenum = vsitenum;
                                            int pos = iinfo * 2 + zz;
                                            int pos1 = iinfo * 2 + 1 + zz;
                                            if(iinfo >= 0 && iinfo < 100)
                                            {
                                                SHORT val = psrc[pos] * 256 + psrc[pos1];
                                                char pvalue[20] = {0};
                                                char pinfo[20] = {0};
                                                char psitenum[20] = {0};
                                                sprintf(pvalue, "%d", val);
                                                sprintf(pinfo, "%d", iinfo);
                                                sprintf(psitenum, "%d", isitenum);
                                                map<string, _variant_t>m_var;
                                                m_var.clear();
                                                string con = "where deplayment=1 and infonum=";
                                                con.append(pinfo);
                                                con.append(" and sitenum=");
                                                con.append(psitenum);
                                                con.append(" and l_comaddr=\'");
                                                con.append(lp_io->gayway);
                                                con.append("\'");
                                                _variant_t  vnumvalue(pvalue);
                                                _variant_t  vdate("getDate()");
                                                m_var.insert(pair<string, _variant_t>("numvalue", vnumvalue));
                                                _variant_t  vday("GETDATE()");
                                                m_var.insert(pair<string, _variant_t>("onlinetime", vday));
                                                string updatesql = dbopen->GetUpdateSql(m_var, "t_sensor", con);
                                                PostLog("传感器:%s", updatesql.c_str());
                                                _RecordsetPtr rsupdate = dbopen->ExecuteWithResSQL(updatesql.c_str());
                                            }
                                            else
                                            {
                                                PostLog("信息点过大:%d", iinfo);
                                            }

                                            rs->MoveNext();
                                        }
                                    }
                                    //控制点
                                    //else if(psrc[2] == packlen - 5 && packlen - 5 == 40)   //控制点采集
                                    //{
                                    //    PostLog("采集控制点数据");
                                    //    int zz = 3;
                                    //    string gayway = lp_io->gayway;
                                    //    string sql = "select * from t_loop where l_deplayment=1 and l_comaddr=\'";
                                    //    sql.append(gayway);
                                    //    sql.append("\' ORDER BY l_info DESC");
                                    //    _RecordsetPtr rs = dbopen->ExecuteWithResSQL(sql.c_str());

                                    //    while(rs && !rs->adoEOF)
                                    //    {
                                    //        variant_t vinfoval = rs->GetCollect("l_info");
                                    //        int iinfo = vinfoval;
                                    //        variant_t vsitenum = rs->GetCollect("l_site");
                                    //        int isitenum = vsitenum;
                                    //        int pos = iinfo * 2 + zz;
                                    //        int pos1 = iinfo * 2 + 1 + zz;
                                    //        if(iinfo >= 0 && iinfo < 20)
                                    //        {
                                    //            SHORT val = psrc[pos] * 256 + psrc[pos1];
                                    //            char pvalue[20] = {0};
                                    //            char pinfo[20] = {0};
                                    //            char psitenum[20] = {0};
                                    //            sprintf(pvalue, "%d", val);
                                    //            sprintf(pinfo, "%d", iinfo);
                                    //            sprintf(psitenum, "%d", isitenum);
                                    //            map<string, _variant_t>m_var;
                                    //            m_var.clear();
                                    //            string con = "where l_deplayment=1 and l_info=";
                                    //            con.append(pinfo);
                                    //            con.append(" and l_site=");
                                    //            con.append(psitenum);
                                    //            con.append(" and l_comaddr=\'");
                                    //            con.append(lp_io->gayway);
                                    //            con.append("\'");
                                    //            _variant_t  vnumvalue(pvalue);
                                    //            m_var.insert(pair<string, _variant_t>("l_switch", vnumvalue));
                                    //            string updatesql = dbopen->GetUpdateSql(m_var, "t_loop", con);
                                    //            PostLog("回路:%s", updatesql.c_str());
                                    //            _RecordsetPtr rsupdate = dbopen->ExecuteWithResSQL(updatesql.c_str());
                                    //        }
                                    //        else
                                    //        {
                                    //            PostLog("信息点过大:%d", iinfo);
                                    //        }
                                    //        rs->MoveNext();
                                    //    }
                                    //}
                                    //场景值
                                    else if(psrc[2] == packlen - 5 && packlen - 5 == 6)
                                    {
                                        SHORT scenenum =  psrc[3] * 256 + psrc[4];

										 SHORT infoval =  psrc[5] * 256 + psrc[6];

										SHORT looval =  psrc[7] * 256 + psrc[8];
                                        PostLog("网关[%s] 场景号:%d", lp_io->gayway, scenenum);
                                        map<string, _variant_t>m_var;
                                        _variant_t  vday("GETDATE()");
                                        _variant_t  vscene((int)scenenum);
                                        _variant_t  vcomaddr(lp_io->gayway);
                                        m_var.insert(pair<string, _variant_t>("day", vday));
                                        m_var.insert(pair<string, _variant_t>("scenenum", vscene));
                                        m_var.insert(pair<string, _variant_t>("comaddr", vcomaddr));
                                        string sql =  dbopen->GetInsertSql(m_var, "t_scene");
                                        PostLog("场景:%s", sql.c_str());
                                        _RecordsetPtr rs = dbopen->ExecuteWithResSQL(sql.c_str());


										//回路
										string gayway = lp_io->gayway;
										string sqlloop = "select * from t_loop where l_deplayment=1 and l_comaddr=\'";
										sqlloop.append(gayway);
										sqlloop.append("\' ORDER BY l_info DESC");
										_RecordsetPtr rsoop = dbopen->ExecuteWithResSQL(sqlloop.c_str());

										while(rsoop && !rsoop->adoEOF)
										{
											variant_t vinfoval = rsoop->GetCollect("l_info");
											int iinfo = vinfoval;
											variant_t vsitenum = rsoop->GetCollect("l_site");
											int isitenum = vsitenum;
											
											BYTE val=looval>>iinfo&1==1?1:0;

											char pvalue[20] = {0};
											char pinfo[20] = {0};
											char psitenum[20] = {0};
											sprintf(pvalue, "%d", val);
											sprintf(pinfo, "%d", iinfo);
											sprintf(psitenum, "%d", isitenum);
											map<string, _variant_t>m_var;
											m_var.clear();
											string con = "where l_deplayment=1 and l_info=";
											con.append(pinfo);
											con.append(" and l_site=");
											con.append(psitenum);
											con.append(" and l_comaddr=\'");
											con.append(lp_io->gayway);
											con.append("\'");
											_variant_t  vnumvalue(pvalue);
											m_var.insert(pair<string, _variant_t>("l_switch", vnumvalue));
											string updatesql = dbopen->GetUpdateSql(m_var, "t_loop", con);
											PostLog("回路:%s", updatesql.c_str());
											_RecordsetPtr rsupdate = dbopen->ExecuteWithResSQL(updatesql.c_str());
											rsoop->MoveNext();

                                    }
                                }
								
                            }
                            else
                            {
                                break;
                            }

                            n += endlen;
                            z++;
                        }
                    }

                    SHORT crc16 = usMBCRC16(psrc1, packlen - 2);
                    SHORT crc16_ = *(SHORT*)(&psrc1[packlen - 2]);

                    if(crc16 == crc16_)
                    {
                        //200 信息点  40:控制点     2:场景
                        if(psrc1[3] == 40 || psrc1[3] == 200 || psrc1[3] == 2)
                        {
                            return FALSE;
                        }

                        PostLog("长度:%d 数据:%s", dwBytes, data.c_str());
                        map<string, list<MSGPACK>>::iterator itmsg = m_MsgPack.find(lp_io->gayway);
                        Json::Value root;

                        if(itmsg != m_MsgPack.end())
                        {
                            list<MSGPACK>v_msg = itmsg->second;

                            if(v_msg.size() > 0)
                            {
                                MSGPACK msgEnd = v_msg.back();
                                IOCP_IO_PTR lp_io1 = NULL;
                                lp_io1 = msgEnd.lp_io;
                                itmsg->second.pop_back();
                                root = msgEnd.root;

                                if(itmsg->second.size() > 0)
                                {
                                    MSGPACK msgEnd = itmsg->second.back();
                                    time_t tmnow = time(NULL);
                                    float t = (float)(tmnow - msgEnd.timestamp) / 60;
                                    PostLog("最后消息队列帧:%d 驻留分钟数%0.2f分", msgEnd.seq, t);

                                    if(t > 1)
                                    {
                                        itmsg->second.clear();
                                    }
                                }

                                if(lp_io1 != NULL)
                                {
                                    root["status"] = "success";
                                    root["comaddr"] = lp_io->gayway;
                                    root["data"] = gstring::char2hex((const char*)psrc1, packlen);
                                    root["length"] = datalen;
                                    string inmsg = root.toStyledString();
                                    char outmsg[1048] = {0};
                                    int lenret = 0;
                                    int len = wsEncodeFrame(inmsg, outmsg, WS_TEXT_FRAME, lenret);

                                    if(len != WS_ERROR_FRAME)
                                    {
                                        memcpy(lp_io1->buf, outmsg, lenret);
                                        lp_io1->wsaBuf.buf = lp_io1->buf;
                                        lp_io1->wsaBuf.len = lenret;
                                        lp_io1->operation = IOCP_WRITE;
                                        DataAction(lp_io1, lp_io1->lp_key);
                                    }
                                }
                            }
                        }
                    }
                }

                //if(checkFlag(psrc, packlen))
                //{
                //    PostThreadMessageA(ThreadId, WM_USER + 1, (WPARAM)lp_io, (LPARAM)0);
                //    lp_io->ibeathit += 1;
                //    return TRUE;
                //    // PostThreadMessageA(ThreadId, WM_USER + 1, (WPARAM)lp_io, (LPARAM)0);
                //}
            }
        }
    }

    if(lp_io->fromtype == SOCKET_FROM_WEBSOCKET)
    {
        int complepack = wsPackCheck(src, dwBytes);
        int alllenth = dwBytes;
        int typepack = 0;
        map<IOCP_IO_PTR, pBREAKPCK>::iterator webite;

        if(complepack == WS_ALL_PACK)
        {
            goto COMPLETEPACK;
        }

        webite =  m_pack.find(lp_io);

        if(webite != m_pack.end())
        {
            pBREAKPCK pack = webite->second;
            AppendByte(src, alllenth, pack, lp_io);
            PostLog("web 断包包尾:lp_io:%p 长度:%d", lp_io, dwBytes);

            if(alllenth > BUFFER_SIZE)
            {
                PostLog("包长度过大:%d", alllenth);

                if(pack)
                {
                    delete pack;
                    pack = NULL;
                }

                m_pack.erase(webite);
                return 1;
            }
        }

        typepack = wsPackCheck(src, alllenth);

        if(typepack == WS_ALL_PACK)
        {
            map<IOCP_IO_PTR, pBREAKPCK>::iterator webite =  m_pack.find(lp_io);

            if(webite != m_pack.end())
            {
                m_pack.erase(webite);
            }

            goto  COMPLETEPACK;
        }
        else if(typepack == WS_BREAK_PACK)
        {
            map<IOCP_IO_PTR, pBREAKPCK>::iterator itepack =  m_pack.find(lp_io);

            if(itepack == m_pack.end())
            {
                //websocket断包处理
                pBREAKPCK webpack = new BREAK_PACK;
                BYTE *b1 = new BYTE[datalen];
                memset(b1, 0, datalen);
                memcpy(b1, src, datalen);
                webpack->b = b1;
                webpack->len = datalen;
                m_pack.insert(make_pair(lp_io, webpack));
                PostLog("web 断包包头:lp_io:%p 长度:%d", lp_io, datalen);
            }
        }

COMPLETEPACK:
        string  strret = "";
        BOOL bBreadPack = FALSE;
        int lenread = wsDecodeFrame(lp_io->buf, strret, alllenth, bBreadPack);
        // PostLog("lenread:%d bBreadPack:%d", lenread, bBreadPack);

        if(lenread == WS_OPENING_FRAME && bBreadPack == FALSE)
        {
            map<IOCP_IO_PTR, pBREAKPCK>::iterator ite1 =  m_pack.find(lp_io);

            if(ite1 != m_pack.end())
            {
                m_pack.erase(ite1);
            }

            DealWebsockMsg(lp_io, lp_key, strret, alllenth);
        }
        else if(lenread == WS_CLOSING_FRAME)
        {
            PostLog("web端退出 通信指针:%p", lp_io);
            lp_io->operation = IOCP_END;
        }

        return 1;
    }

    return 1;
}

int CIOCP::wsPackCheck(BYTE src[], int len)
{
    int ret = FALSE;
    const char *msg = (const char*)src;
    const int frameLength = len;

    if(frameLength < 2)
    {
        return FALSE;
    }

    BYTE Fin =  msg[0] >> 7 & 1;
    BYTE RSV1 = msg[0] >> 6 & 0x01;
    BYTE RSV2 = msg[0] >> 5 & 0x01;
    BYTE RSV3 = msg[0] >> 4 & 0x01;
    BYTE Mask =  msg[1] >> 7 & 0x01;

    //FIN:1位，用于描述消息是否结束，如果为1则该消息为消息尾部,如果为零则还有后续数据包;
    if(Fin != 1)
    {
        return ret;
    }

    // 检查扩展位并忽略
    if(RSV1 == 1 || RSV2 == 1 || RSV3 == 1)
    {
        return ret;
    }

    // mask位, 为1表示数据被加密
    if(Mask != 1)
    {
        return ret;
    }

    BYTE opcode = msg[0] & 0x0f;
    // 操作码
    uint16_t payloadLength = 0;
    uint8_t payloadFieldExtraBytes = 0;

    if(opcode == WS_TEXT_FRAME)
    {
        // 处理utf-8编码的文本帧
        payloadLength = static_cast<uint16_t >(msg[1] & 0x7f);

        if(payloadLength == 0x7e)   //0111 1110     //126 7e  后面两字节是长度 :  127  7f 后面四字节是长度
        {
            uint16_t payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &msg[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }
        else if(payloadLength == 0x7f)
        {
            // 数据过长,暂不支持
            uint32_t payloadLength32b = 0;
            payloadFieldExtraBytes = 4;
            memcpy(&payloadLength32b, &msg[2], payloadFieldExtraBytes);
            payloadLength = ntohl(payloadLength32b);
            //ret = WS_ERROR_FRAME;
        }

        if(payloadLength == (len - 2 - payloadFieldExtraBytes - 4))
        {
            return WS_ALL_PACK;
        }

        if(payloadLength > len - 2 - payloadFieldExtraBytes - 4)
        {
            return WS_BREAK_PACK;
        }
    }

    return WS_ERROR_PACK;
}
