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

//���캯��
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
            Shell_NotifyIcon(NIM_ADD, &nid); //�����������ͼ��de����
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
                gstring::tip("�빴ѡ����");
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
                PostLog("����˻�ĩ����");
                return;
            }

            //glog::trace("come on CheckForInvalidConnection");
            while(lp_start != NULL)
            {
                if(lp_start->fromtype == SOCKET_FROM_GAYWAY)
                {
                    op_len = sizeof(op);
                    nRet = getsockopt(lp_start->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);
                    int len = 0;

                    if(op != 0xffffffff)
                    {
                        len = op - lp_start->timelen;
                    }

                    PostLog("����:%s ���߼���յ�����Ϣ:%d�� ͨ��ָ��:%p", lp_start->gayway, len, lp_start);
                }

                lp_start = m_io_group.GetNext(pos);
            }

            PostLog("��������:%d", m_mcontralcenter.size());
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
                PostLog("��ѡ���б�");
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
            string data =   getItemText(m_plistuser, m_plistuser->GetCurSel(), 4);
            IOCP_IO_PTR io = (IOCP_IO_PTR)strtol(lpiostr.c_str(), NULL, 16);
            IOCP_KEY_PTR ik = (IOCP_KEY_PTR)strtol(lpiostr.c_str(), NULL, 16);

            if(io == 0 || ik == 0)
            {
                PostLog("��ѡ���б�");
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
            PostLog("���͵�ָ����:%s", str.c_str());
            int n = m_plistuser->GetCurSel();

            if(n == -1)
            {
                PostLog("�빴ѡ�б�");
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

//��������
/*-------------------------------------------------------------------------------------------
�������ܣ��رղ������Դ
����˵����
�������أ�
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
    �������ܣ���ʼ��IO���
    ����˵����
    �������أ�
    -------------------------------------------------------------------------------------------*/
void CIOCP::InitIoContext(IOCP_IO_PTR lp_io)
{
    memset(&lp_io->ol,  0, sizeof(WSAOVERLAPPED));
    memset(&lp_io->buf, 0, BUFFER_SIZE);
    lp_io->wsaBuf.buf       = lp_io->buf;
    lp_io->wsaBuf.len       = BUFFER_SIZE;
}

/*-------------------------------------------------------------------------------------------
    �������ܣ���ʼ������SOCKET�˿ڣ�������ɶ˿�����������
    ����˵����
    �������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
    �������ܣ��ر������߳�
    ����˵����
    �������أ�
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
�������ܣ��������˿ں��Լ���IP��PORT�󶨣�����ʼ����
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
�������ܣ�����CPU����Ŀ��������Ӧ���������ݴ����߳�
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
�������ܣ�����һ������������
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
        lp_io->ibreakpack = 0;
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
�������ܣ��������ݺ���
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
                glog::GetInstance()->AddLine("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
                PostLog("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
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
                glog::trace("����ip:%s Զ��ip:%s", szLocalAddress, szPeerAddress);
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
                // glog::GetInstance()->AddLine("������:%d ������:%s", dwByte, data.c_str());
                pListElement->SetText(5, data.c_str());
                pListElement->SetText(6, lp_io->buf);
                char lenstr[20] = {0};
                sprintf(lenstr, "%d", dwByte);
                pListElement->SetText(7, lenstr);
                glog::GetInstance()->AddLine("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
                PostLog("�ͻ�������:%s lp_io:%p     lp_key:%p", szPeerAddress, lp_io, lp_key);
                string req = lp_io->buf;
                string res;
                int wsconn = wsHandshake(req, res);

                if(wsconn == WS_STATUS_CONNECT)
                {
                    InitIoContext(lp_io);
                    //lp_io->operation = IOCP_WRITE;
                    lp_io->fromtype = SOCKET_FROM_WEBSOCKET;
                    pListElement->SetText(8, "web�ͻ���(2)");
                    strcpy(lp_io->gayway, "web�ͻ���(2)");
                    memcpy(lp_io->buf, res.c_str(), res.size());
                    PostLog("web������....");
                    lp_io->wsaBuf.len = res.size();
                    lp_io->operation = IOCP_WRITE;
                    break;
                }
                else
                {
     

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
/*�������ܣ�����һЩ�ص�����
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
                //PostLog(" DataAction->IOCP_END  �ر�socket:%p   ", lp_io);
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
�������ܣ��õ�MS��װ��SOCKET����ָ�룬������������ٶ�
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
�������ܣ�ע��FD_ACCEPTG�¼���m_h_accept_event�¼����Ա����з���ȥ�����Ӻĺľ�ʱ���õ�֪ͨ��
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
�������ܣ��õ����������Ŀͻ���IP��PORT
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::GetAddrAndPort(char*buf, char ip[], UINT & port)
{
    int     len = BUFFER_SIZE - sizeof(SOCKADDR_IN) - 16;
    char    *lp_buf = buf + len;    //ֱ�Ӷ�ȡԶ�˵�ַ
    SOCKADDR_IN addr;
    memcpy(&addr, lp_buf, sizeof(SOCKADDR_IN));
    port    = ntohl(addr.sin_port);
    strcpy(ip, inet_ntoa(addr.sin_addr));
    MSG("�ͻ�IPΪ��");
    MSG(ip);
    MSG("�ͻ��˿�Ϊ��");
    MSG(port);
    return TRUE;
}

void CIOCP::DealWebsockMsg(IOCP_IO_PTR& lp_io, IOCP_KEY_PTR& lp_key, string jsondata, int len)
{
    PostLog("����:%d web������:%s ", len, jsondata.c_str());
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
                if(msgType == "AA" || msgType == "A4" || msgType == "A5" || msgType == "AC" || msgType == "00" || msgType == "FE" || msgType == "FF")
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
                            SHORT crc16 = usMBCRC16(bitSend, len);
                            bitSend[len] = crc16 & 0xff;
                            bitSend[len + 1] = crc16 >> 8 & 0xff;
                            int sendlen = len + 2;
                            string tosenddata = gstring::char2hex((const char*)bitSend, sendlen);
                            PostLog("ת����:%s ����:%d", tosenddata.c_str(), sendlen);
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
        if(msg.message == WM_USER + 1)
        {
            //__try
            //  {
            IOCP_IO_PTR lo = (IOCP_IO_PTR)msg.wParam;
            // lp_this->PostLog("%X %X", lo, msg.lParam);

            if(lo)
            {
                time_t tmtamp;
                struct tm *tm1 = NULL;
                time(&tmtamp) ;
                tm1 = localtime(&tmtamp) ;
                tm1->tm_yday;
                BYTE ii = msg.lParam;
                int day = (int)(ii >> 4 & 0xf) * 10 + (int)(ii & 0xf);

                if(tm1->tm_mday == day)
                {
                    tm1->tm_mday--;
                    mktime(tm1);
                    char myday[30] = {0};
                    strftime(myday, sizeof(myday), "%Y-%m-%d", tm1);
                    char gayway[20] = {0};
                    strcpy(gayway, lo->gayway);

                    if(_stricmp(lo->day, myday) != 0)
                    {
                        vector<BYTE>v_b;
                        int n = 0;

                        if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                            //���������ѹ
                            lp_this->PostLog("����[%s] �������������ѹ����", gayway);
                            //unsigned char vol[24] = {0x68, 0x42, 0x00, 0x42, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x7A, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x01, 0x05, 0x55, 0x16};
                            BYTE vol[50] = {0};
                            n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x404, v_b, vol);
                            lp_this->InitIoContext(lo);
                            memcpy(lo->buf, vol, n);
                            lo->wsaBuf.len = n; // sizeof(vol);
                            lo->wsaBuf.buf = lo->buf;
                            lo->operation = IOCP_WRITE;
                            lp_this->DataAction(lo, lo->lp_key);
                            //�����������
                            Sleep(10000);
                        }

                        if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                            lp_this->PostLog("����[%s]�������������������", gayway);
                            BYTE electric[50] = {0}; //{0x68, 0x32, 0x00, 0x32, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x75, 0x00, 0x00, 0x20, 0x04, 0x66, 0x16 };
                            n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x420, v_b, electric);
                            string data1 = gstring::char2hex((const char*)electric, n);
                            //glog::GetInstance()->AddLine("�������Ͱ�:%s", data1.c_str());
                            lp_this->InitIoContext(lo);
                            memcpy(lo->buf, electric, n);
                            lo->wsaBuf.len = n;
                            lo->wsaBuf.buf = lo->buf;
                            lo->operation = IOCP_WRITE;
                            lp_this->DataAction(lo, lo->lp_key);
                            //���������й�����
                            Sleep(10000);
                        }

                        if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                            lp_this->PostLog("����[%s]�������������й���������", gayway);
                            unsigned char activepower[50] = {0}; // {0x68, 0x42, 0x00, 0x42, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x76, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x20, 0x04, 0x6B, 0x16 };
                            n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x301, v_b, activepower);
                            lp_this->InitIoContext(lo);
                            memcpy(lo->buf, activepower, n);
                            lo->wsaBuf.len = n; //sizeof(activepower);
                            lo->wsaBuf.buf = lo->buf;
                            lo->operation = IOCP_WRITE;
                            lp_this->DataAction(lo, lo->lp_key);
                            ////�����ܹ�������
                            Sleep(10000);
                        }

                        if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                            lp_this->PostLog("����[%s]�������칦������", gayway);
                            unsigned char powerfactor[50] = {0}; //{0x68, 0x42, 0x00, 0x42, 0x00, 0x68, 0x04, comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x78, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0x20, 0x04, 0xAC, 0x16 };
                            n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x340, v_b, powerfactor);
                            lp_this->InitIoContext(lo);
                            memcpy(lo->buf, powerfactor, n);
                            lo->wsaBuf.len = n;
                            lo->wsaBuf.buf = lo->buf;
                            lo->operation = IOCP_WRITE;
                            lp_this->DataAction(lo, lo->lp_key);
                            //��������
                            Sleep(10000);
                        }

                        if(lp_this->m_mcontralcenter.find(gayway) != lp_this->m_mcontralcenter.end())
                        {
                            lp_this->PostLog("����[%s]����������������", gayway);
                            unsigned char power[50] = {0};//{0x68, 0x32, 0x00, 0x32, 0x00, 0x68, 0x04,  comaddr[1], comaddr[0], comaddr[3], comaddr[2], 0x02, 0xAC, 0x7B, 0x00, 0x00, 0x01, 0x05, 0x4E, 0x16 };
                            n = lp_this->buidByte(gayway, 0x4, 0xAC, 0x71, 0, 0x501, v_b, power);
                            lp_this->InitIoContext(lo);
                            memcpy(lo->buf, power, n);
                            lo->wsaBuf.len = n;
                            lo->wsaBuf.buf = lo->buf;
                            lo->operation = IOCP_WRITE;
                            lp_this->DataAction(lo, lo->lp_key);
                            strcpy(lo->day, myday);
                        }
                    }
                }
            }

            //  }
            //__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            //  {
            //  }
        }
        else if(msg.message == WM_USER + 2)
        {
            if(msg.wParam == 1000 && msg.lParam == 1000)
            {
                lp_this->PostLog("�������ݿ�");
                string pdir = lp_this->GetDataDir("config.ini");
                lp_this->dbopen->ReConnect(pdir);
            }
        }
        else if(msg.message == WM_USER + 3)
        {
            string pid = (char*)msg.wParam;
            lp_this->PostLog("projectId:%s", pid.c_str());
            string sql = "SELECT l_comaddr,l_code FROM   t_lamp tl\
					   WHERE  l_deplayment = 1 AND l_comaddr IN (SELECT comaddr AS l_comaddr FROM    t_baseinfo WHERE  online=1 AND  pid     = \'";
            sql.append(pid);
            sql.append("\') GROUP BY tl.l_comaddr,tl.l_code ");
            glog::trace("%s", sql.c_str());
            _RecordsetPtr rs = lp_this->dbopen->ExecuteWithResSQL(sql.c_str());
            map<string, list<SHORT>>m_lamp;

            while(rs && !rs->adoEOF)
            {
                try
                {
                    _variant_t l_comaddr = rs->GetCollect("l_comaddr");
                    _variant_t l_code = rs->GetCollect("l_code");
                    string comaddr = _com_util::ConvertBSTRToString(l_comaddr.bstrVal);
                    map<string, list<SHORT>>::iterator it = m_lamp.find(comaddr);

                    if(it != m_lamp.end())
                    {
                        it->second.push_back(l_code);
                    }
                    else
                    {
                        list<SHORT>v_l_code;
                        v_l_code.push_back(l_code);
                        m_lamp.insert(pair<string, list<SHORT>>(comaddr, v_l_code));
                    }

                    rs->MoveNext();
                }
                catch(_com_error e)
                {
                    break;
                }
            }

            if(m_lamp.size() > 0)
            {
                string sql1 = "UPDATE t_lamp SET presence = 0 WHERE  l_deplayment = 1 AND l_comaddr IN (SELECT comaddr AS l_comaddr FROM   t_baseinfo WHERE  pid = \'";
                sql1.append(pid);
                sql1.append("\'");
                sql1.append(")");
                _RecordsetPtr rs = lp_this->dbopen->ExecuteWithResSQL(sql1.c_str());
            }

            for(auto it = m_lamp.begin(); it != m_lamp.end(); it++)
            {
                list<SHORT>v_s = it->second;
                string l_comaddr = it->first;
                vector<BYTE>v_param;
                int z = 0;

                for(auto it = v_s.begin(); it != v_s.end();)
                {
                    SHORT s = *it;

                    if(z == 0)
                    {
                        v_param.push_back(v_s.size());
                    }

                    v_s.erase(it++);
                    z++;
                    BYTE a =  s >> 8 & 0x00ff;
                    BYTE b =  s & 0x00ff;
                    v_param.push_back(b);
                    v_param.push_back(a);

                    if(it == v_s.end())
                    {
                        BYTE vol[1024] = {0};
                        int  n = lp_this->buidByte(l_comaddr, 0x4, 0xAC, 0x71, 0, 0x0040, v_param, vol);
                        string  hh = gstring::char2hex((char*)vol, n);
                        glog::GetInstance()->AddLine("%s", hh.c_str());
                        map<string, IOCP_IO_PTR>::iterator itegay = lp_this->m_mcontralcenter.find(l_comaddr);

                        if(itegay != lp_this->m_mcontralcenter.end())
                        {
                            IOCP_IO_PTR lo = itegay->second;
                            lp_this->InitIoContext(lo);
                            memcpy(lo->buf, vol, n);
                            lo->wsaBuf.len = n; // sizeof(vol);
                            lo->wsaBuf.buf = lo->buf;
                            lo->operation = IOCP_WRITE;
                            lp_this->DataAction(lo, lo->lp_key);
                            Sleep(100);
                        }

                        break;
                    }

                    if(z == 50)
                    {
                        BYTE vol[1024] = {0};
                        v_param[0] = 50;
                        int  n = lp_this->buidByte(l_comaddr, 0x4, 0xAC, 0x71, 0, 0x0040, v_param, vol);
                        string  hh = gstring::char2hex((char*)vol, n);
                        glog::GetInstance()->AddLine("%s", hh.c_str());
                        map<string, IOCP_IO_PTR>::iterator itegay = lp_this->m_mcontralcenter.find(l_comaddr);

                        if(itegay != lp_this->m_mcontralcenter.end())
                        {
                            IOCP_IO_PTR lo = itegay->second;
                            lp_this->InitIoContext(lo);
                            memcpy(lo->buf, vol, n);
                            lo->wsaBuf.len = n; // sizeof(vol);
                            lo->wsaBuf.buf = lo->buf;
                            lo->operation = IOCP_WRITE;
                            lp_this->DataAction(lo, lo->lp_key);
                            Sleep(100);
                        }

                        z = 0;
                    }
                }
            }
        }
    }

    return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*-------------------------------------------------------------------------------------------
�������ܣ���ʼ����ɶ˿ڼ���ص����ж�����������ÿһ��10������.
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::InitAll()
{
    CoInitialize(NULL);
    //------------------------------------------------------------
    //-----------       Created with 010 Editor        -----------
    //------         www.sweetscape.com/010editor/          ------
    //
    // File    : Untitled3
    // Address : 0 (0x0)
    // Size    : 173 (0xAD)
    //------------------------------------------------------------
    /* unsigned char hexData[173] =
     {
       0xF3, 0xFD, 0x54, 0x8B, 0x90, 0xE5, 0x5A, 0xE8, 0xA7, 0xA6, 0x1A, 0xE8, 0xEB, 0xF7, 0x5A, 0xE8,
       0xA5, 0xBE, 0x06, 0xAF, 0xF3, 0xFD, 0x46, 0xE6, 0xF3, 0xA3, 0x17, 0xBE, 0xB0, 0xE5, 0x4C, 0x91,
       0xE3, 0xF7, 0x5A, 0xF8, 0xE1, 0xEB, 0x44, 0xFA, 0xFD, 0xF5, 0x46, 0xE6, 0xE3, 0xF7, 0x5A, 0xF8,
       0xE1, 0xEB, 0x44, 0xFA, 0xFD, 0xF5, 0x46, 0xE6, 0xE3, 0xF7, 0x5A, 0xF8, 0xE1, 0xEB, 0x44, 0xFA,
       0xFD, 0xF5, 0x46, 0xE6, 0xE3, 0xF7, 0x5A, 0xF8, 0xE1, 0xEB, 0x44, 0xFA, 0xFD, 0xF5, 0x46, 0xE6,
       0xE3, 0xF7, 0x5A, 0xF8, 0xE1, 0xEB, 0x44, 0xFA, 0xFD, 0xF5, 0x46, 0xE6, 0xE3, 0xF7, 0x5A, 0xF8,
       0xE1, 0xEB, 0x44, 0xFA, 0xFD, 0xF5, 0x46, 0xE6, 0xE3, 0xF7, 0x5A, 0xF8, 0xE1, 0xEB, 0x44, 0xFA,
       0xFD, 0xF5, 0x46, 0xE6, 0xE3, 0xF7, 0x5A, 0xF8, 0xE1, 0xEB, 0x44, 0xFA, 0xFD, 0xF5, 0x46, 0xE6,
       0xE3, 0xF7, 0x5A, 0xF8, 0xE1, 0xEB, 0x44, 0xFA, 0xFD, 0xF5, 0x46, 0xE6, 0xE3, 0xF7, 0x5A, 0xF8,
       0xE1, 0xEB, 0x44, 0xFA, 0xFD, 0xF5, 0x46, 0x97, 0xFD, 0xE5, 0x1A, 0xAF, 0xBF, 0xE5, 0x4C, 0xFE,
       0xE1, 0xEB, 0x54, 0xAF, 0xBF, 0xA3, 0x54, 0xF0, 0xF3, 0xF1, 0x37, 0xE8, 0xAC
     };
     string str = "";
     string str1 = "";
     wsHandshake(str, str1);
     string out = "";
     BOOL fullpack = FALSE;
     wsDecodeFrame((char*)hexData, out, 16, fullpack);*/
    //objeamil.SetEmailTitle(string("aaaa"));
    //objeamil.AddTargetEmail(string("277402131@qq.com"));
    //objeamil.SetContent(string("asdfsdfsdfsdfsdf"));
    //objeamil.SendVecotrEmail();
    //CSmtp smtp(25, "smtp.126.com", "z277402131@126.com", /*��������ַ*/"z277402131",/*��������*/"zhizhuchun@qq.com",/*Ŀ�������ַ*/"TEST",/*����*/"���Բ��ԣ��յ���ظ���"  /*�ʼ�����*/);
    //string filePath("D:\\����.txt");
    //smtp.AddAttachment(filePath);
    /*�����Ե���CSmtp::DeleteAttachment����ɾ������������һЩ�������Լ���ͷ�ļ���!*/
    //int err;
    //if((err = smtp.SendEmail_Ex()) != 0)
    //  {
    //    if(err == 1)
    //      cout << "����1: �������粻��ͨ������ʧ��!" << endl;
    //    if(err == 2)
    //      cout << "����2: �û�������,��˶�!" << endl;
    //    if(err == 3)
    //      cout << "����3: �û����������˶�!" << endl;
    //    if(err == 4)
    //      cout << "����4: ���鸽��Ŀ¼�Ƿ���ȷ���Լ��ļ��Ƿ����!" << endl;
    //  }
// return 0;
    //string strTarEmail = "12345678@qq.com";
    //smtp.AddTargetEmail(strTarEmail);
    //if((err = smtp.SendVecotrEmail()) != 0) {
    //    if(err == -1)
    //        cout << "����-1: û��Ŀ�������ַ!" << endl;
    //    if(err == 1)
    //        cout << "����1: �������粻��ͨ������ʧ��!" << endl;
    //    if(err == 2)
    //        cout << "����2: �û�������,��˶�!" << endl;
    //    if(err == 3)
    //        cout << "����3: �û����������˶�!" << endl;
    //    if(err == 4)
    //        cout << "����4: ���鸽��Ŀ¼�Ƿ���ȷ���Լ��ļ��Ƿ����!" << endl;
    //}
    //------------------------------------------------------------
    //-----------       Created with 010 Editor        -----------
    //------         www.sweetscape.com/010editor/          ------
    //
    // File    : Untitled1
    // Address : 0 (0x0)
    // Size    : 63 (0x3F)
    //------------------------------------------------------------
    //unsigned char hexData[63] =
    //{
    //  0xB8, 0x04, 0x0E, 0x22, 0xA9, 0x14, 0x0C, 0x33, 0xE8, 0x11, 0x1E, 0x24, 0xB9, 0x04, 0x0E, 0x23,
    //  0xA9, 0x14, 0x0E, 0x33, 0xB9, 0x15, 0x1E, 0x23, 0xBA, 0x04, 0x0D, 0x25, 0xA9, 0x14, 0x0E, 0x33,
    //  0xB9, 0x14, 0x1E, 0x24, 0xB9, 0x04, 0x0F, 0x25, 0xA9, 0x06, 0x12, 0x31, 0xE5, 0x41, 0x50, 0x31,
    //  0xB3, 0x12, 0x07, 0x3F, 0xAB, 0x41, 0x50, 0x77, 0xAB, 0x1E, 0x1C, 0x25, 0xC8, 0x06, 0x43
    //};
    //string outstring = "";
    //BOOL bfullpack = TRUE;
    //int n1 = wsDecodeFrame((char*)hexData, outstring, sizeof(hexData), bfullpack);
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

    //��ʱ�ɼ��߳�
    //DWORD tid = 0;
    HANDLE hTreadTime = CreateThread(NULL, NULL, TimeThread, (LPVOID)this, NULL, &ThreadId);
    CloseHandle(hTreadTime);
    ////��ʱ�ɼ��߳�
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
�������ܣ���ѭ��
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
-------------------------------------------------------------------------------------------*/
BOOL CIOCP::MainLoop()
{
    DWORD   dwRet;
    int     nCount = 0;
    PostLog("���������...");
    //cout << "Server is running.........." << nCount++ << " times" << endl;
    int ii = 0;

    while(TRUE)
    {
        dwRet = WaitForSingleObject(m_h_accept_event, INFINITE);

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
                    //��⼯������ʱ����
                    //cout << "Server is running.........." << nCount++ << " times" << endl;
                    CheckForInvalidConnection();
                }
                break;

            case WAIT_OBJECT_0:   //���յ������з��������Ӷ��ù��˵���Ϣ���ٴη�������
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
�������ܣ������Ƿ��������ˣ����ܳ�ʱ��û�����ݵġ���Ч���ӡ����еĻ������ߵ�
����˵����
�������أ��ɹ���TRUE��ʧ�ܣ�FALSE
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
        op_len = sizeof(op);
        nRet = getsockopt(lp_start->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);

        if(SOCKET_ERROR == nRet)
        {
            glog::traceErrorInfo("CheckForInvalidConnection getsockopt", WSAGetLastError());
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
                    // map<string, IOCP_IO_PTR>::iterator  it; m_mcontralcenter.find(lp_start)
                    glog::GetInstance()->AddLine("ͨ��ָ��:%p ����:%s ��ʱ%d�� �����ر� ��������:%d", lp_start, lp_start->gayway, len, m_io_group.GetCount());
                    //string sql = "update t_baseinfo set online=0 where comaddr=\'";
                    //sql.append(lp_start->gayway);
                    //sql.append("\'");
                    //glog::trace("\n%s", sql.c_str());
                    //_RecordsetPtr rs =   dbopen->ExecuteWithResSQL(sql.c_str());
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

                    //for(it = m_mcontralcenter.begin(); it != m_mcontralcenter.end();)
                    //{
                    //    if(it->second == lp_start)
                    //    {
                    //        m_mcontralcenter.erase(it++);   //erase ɾ����ָ����һ��������
                    //    }
                    //    else
                    //    {
                    //        it++;
                    //    }
                    //}
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

                if(len / 60 >= 30)
                {
                    glog::GetInstance()->AddLine("�����ر���ҳ�ͻ���");
                    closesocket(lp_start->socket);
                    break;
                }

                //��ҳ20���������ɵ�
            }
            else if(lp_start->fromtype == SOCKET_FROM_UNKNOW)
            {
                int len = op - lp_start->timelen;

                if(len / 60 >= 5)
                {
                    glog::GetInstance()->AddLine("�����ر�ĩ֪�ͻ���");
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
�������ܣ����ݴ����̺߳���
����˵����
�������أ�
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

        //�˳�����
        if(dwBytes == 0 && lp_io->operation != IOCP_ACCEPT)
        {
            lp_this->ExitSocket(lp_io, lp_key, WSAGetLastError());
            glog::GetInstance()->AddLine("�˳� ErrorInfo:%s", lp_this->getErrorInfo(WSAGetLastError()).c_str());
            continue;
        }

        //socket ͨ��ʱ��
        //int op_len = 0;
        //int op = 0;
        //op_len = sizeof(op);
        //nRet = getsockopt(lp_io->socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&op, &op_len);
        //if(SOCKET_ERROR == nRet)
        //  {
        //    lp_this->PostLog("lp_io:%p errorcode:%d getsockopt", lp_io, WSAGetLastError(), lp_this->m_io_group.GetCount());
        //    closesocket(lp_io->socket);
        //    //continue;
        //  }
        //if(op != 0xffffffff)
        //  {
        //    lp_io->timelen = op;
        //    //glog::traceErrorInfo("getsockopt",WSAGetLastError());
        //    //glog::trace("\nlp_io:%p timelen:%d",lp_io,lp_io->timelen);
        //  }
        EnterCriticalSection(&lp_this->crtc_sec);

        if(bRet && lp_io && lp_key)
        {
            string hexdata = gstring::char2hex(lp_io->buf, dwBytes);
            glog::GetInstance()->AddLine("operation:%d ������:%d ������:%s", lp_io->operation, dwBytes, hexdata.c_str());

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
                            glog::traceErrorInfo("GetBlank��", WSAGetLastError());
                            closesocket(lp_io->socket);
                            lp_this->m_io_group.RemoveAt(lp_io);
                            continue;
                        }

                        lp_new_key->socket = lp_io->socket;
                        lp_io->lp_key = lp_new_key;
                        //���½�����SOCKETͬ��ɶ˿ڹ���������
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

                        //�����ȡ��������
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
                        lp_this->dealRead(lp_io, lp_key, dwBytes);
ToMsg:
                        bRet = lp_this->DataAction(lp_io, lp_new_key);
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

        LeaveCriticalSection(&lp_this->crtc_sec);
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
    // ����http����ͷ��Ϣ
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

    // ���http��Ӧͷ��Ϣ
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

    //FIN:1λ������������Ϣ�Ƿ���������Ϊ1�����ϢΪ��Ϣβ��,���Ϊ�����к������ݰ�;
    if(Fin != 1)
    {
        ret = WS_ERROR_FRAME;
        return ret;
    }

    // �����չλ������
    if(RSV1 == 1 || RSV2 == 1 || RSV3 == 1)
    {
        ret = WS_ERROR_FRAME;
        return ret;
    }

    // maskλ, Ϊ1��ʾ���ݱ�����
    if(Mask != 1)
    {
        ret = WS_ERROR_FRAME;
        return ret;
    }

    BYTE opcode = msg[0] & 0x0f;
    // ������
    uint16_t payloadLength = 0;
    uint8_t payloadFieldExtraBytes = 0;

    if(opcode == WS_TEXT_FRAME)
    {
        // ����utf-8������ı�֡
        payloadLength = static_cast<uint16_t >(msg[1] & 0x7f);

        if(payloadLength == 0x7e)   //0111 1110     //126 7e  �������ֽ��ǳ��� :  127  7f �������ֽ��ǳ���
        {
            uint16_t payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &msg[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }
        else if(payloadLength == 0x7f)
        {
            // ���ݹ���,�ݲ�֧��
            uint32_t payloadLength32b = 0;
            payloadFieldExtraBytes = 4;
            memcpy(&payloadLength32b, &msg[2], payloadFieldExtraBytes);
            payloadLength = ntohl(payloadLength32b);
            //ret = WS_ERROR_FRAME;
        }
    }
    else if(opcode == WS_BINARY_FRAME || opcode == WS_PING_FRAME || opcode == WS_PONG_FRAME)
    {
        // ������/ping/pong֡�ݲ�����
    }
    else if(opcode == WS_CLOSING_FRAME)
    {
        ret = WS_CLOSING_FRAME;
    }
    else
    {
        ret = WS_ERROR_FRAME;
    }

    // ���ݽ���
    if((ret != WS_ERROR_FRAME) && (payloadLength > 0))
    {
        if(payloadLength > (len - 2 - payloadFieldExtraBytes - 4))
        {
            bBreakPack = TRUE;
            return ret;
        }

        // header: 2�ֽ�, masking key: 4�ֽ�
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
        // �ݲ�֧����ô��������
        return WS_ERROR_FRAME;
    }

    uint16_t payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
    // header: 2�ֽ�, maskλ����Ϊ0(������), ������masking key������д, ʡ��4�ֽ�
    uint8_t frameHeaderSize = 2 + payloadFieldExtraBytes;
    uint8_t *frameHeader = new uint8_t[frameHeaderSize];
    memset(frameHeader, 0, frameHeaderSize);
    // finλΪ1, ��չλΪ0, ����λΪframeType
    frameHeader[0] = static_cast<uint8_t>(0x80 | frameType);

    // ������ݳ���
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

    // �������
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
    if(len < 6)
    {
        return FALSE;
    }

    if(vv[0] == 0x68 && vv[5] == 0x68 && vv[len - 1] == 0x16)
    {
        SHORT len1 = *(SHORT*)&vv[1];
        SHORT len2 = *(SHORT*)&vv[3];
        SHORT len11 = len1 >> 2;
        SHORT len22 = len2 >> 2;

        if(len11 == len22 && len22 == len - 8)
        {
            return TRUE;
        }
    }

    return FALSE;
}
//������Ӧ����  src
/*
*  src Դ�յ������ݰ�
*  srclen Դ������
*  des  ����Ŀ��İ�
*/
void CIOCP::buildcode(BYTE src[], int srclen, IOCP_IO_PTR & lp_io)
{
    //��·��� ��½  ������ c4: 1100 0100  �����룺0x02 src[13] da1 src[14] da2 src[15] dt0    p0  f1  ��½   6������  13֡���� 12
    //&&src[14]==0x0&&src[15]==0x0&&src[16]==0x01&&src[17]==1   1100 0000 1100 0000   1 dir  1 yn PRM  6������  ֡���Ƿ�Ҫ�ظ� src[13]
    BYTE AFN = src[12];
    char addr1[4] = {0};
    memcpy(addr1, &src[7], 4); //��ַ
    char addrarea[20] = {0};
    sprintf(addrarea, "%02x%02x%02x%02x", addr1[1], addr1[0], addr1[3], addr1[2]); //���ص�ַ
    BYTE frame = src[13] & 0x0f;   //��
    BYTE    con =    src[13] & 0x10;
    BYTE   DirPrmCode = src[6] & 0xc0;   //����  �Ӷ�
    BYTE   FC = src[6] & 0xF; //���������Ĺ�����
    BYTE DA[2] = {0};
    BYTE DT[2] = {0};
    //string addrarea = gstring::char2hex(addr1, 4);
    memcpy(DA, &src[14], 2);    //PN P0
    memcpy(DT, &src[16], 2);   //F35  �����ѹ   00 00 04 04 ���������ѹ

    //��·���
    if(AFN == 0x02)      //��·���
    {
        //src[6] == 0xc4 && src[13] & 0x10 == 0x10
        BYTE    con =    src[13] & 0x10;
        BYTE   DirPrmCode = src[6] & 0xc4;
        BYTE DA[2] = {0};
        BYTE DT[2] = {0};
        memcpy(DA, &src[14], 2);    //PN P0
        memcpy(DT, &src[16], 2);   //FN  1 ��½ | 3 ����

        if(DirPrmCode == 0xc4 && con == 0x10)    //��Ҫ�ظ�
        {
            if(DA[0] == 0 && DA[1] == 0 && DT[1] == 0 && DT[0] == 1) //DT1��
            {
                lp_io->fromtype = SOCKET_FROM_GAYWAY;
                glog::GetInstance()->AddLine("����[%s] ��½", addrarea);
                PostLog("����[%s] ��½", addrarea);
                strcpy(lp_io->gayway, addrarea);
                setOnline(addrarea, 1);
                map<string, IOCP_IO_PTR>::iterator it = m_mcontralcenter.find(addrarea);

                if(it == m_mcontralcenter.end())
                {
                    m_mcontralcenter.insert(pair<string, IOCP_IO_PTR>(addrarea, lp_io));
                }
                else
                {
                    it->second = lp_io;
                }

                BYTE des[50] = {0};
                int deslen = 0;
                buildConCode(src, des, deslen, 1);
                InitIoContext(lp_io);
                memcpy(lp_io->buf, des, deslen);
                lp_io->wsaBuf.len = deslen;
                lp_io->operation = IOCP_WRITE;
            }
            else if(DA[0] == 0 && DA[1] == 0 && DT[1] == 0 && DT[0] == 4)
            {
                map<string, IOCP_IO_PTR>::iterator it = m_mcontralcenter.find(addrarea);

                if(it != m_mcontralcenter.end())
                {
                    BYTE day = src[22];
                    PostLog("����[%s] ����", addrarea);
                    BYTE des[50] = {0};
                    int deslen = 0;
                    buildConCode(src, des, deslen, 1);
                    InitIoContext(lp_io);
                    memcpy(lp_io->buf, des, deslen);
                    lp_io->wsaBuf.len = deslen;
                    lp_io->operation = IOCP_WRITE;
                    PostThreadMessageA(ThreadId, WM_USER + 1, (WPARAM)lp_io, (LPARAM)day);
                }
            }
        }
    }
}
void CIOCP::buildConCode(BYTE src[], BYTE res[], int& len, BYTE bcon)
{
    BYTE frame = src[13] & 0xf;   //֡���
    BYTE btemp[216] = {0};
    btemp[0] = 0x68;
    int nbyte1 = 20 - 2 - 6;     //20�Զ��峤��
    short n111 = (nbyte1 << 2) | 2;
    memcpy(&btemp[1], &n111, 2);
    memcpy(&btemp[3], &n111, 2);
    btemp[5] = 0x68;
    btemp[6] = 0x04;                //������ 0000 0100   0100����,�Ӷ�      04�Ƿ��ͻ�ȥ����Ӧ��
    memcpy(&btemp[7], &src[7], 5);  //��ַ��
    btemp[12] = 0x00;
    btemp[13] = frame;
    btemp[19] = 0x16;
    memcpy(&btemp[14], &src[14], 4);
    BYTE  checksum = 0;

    for(int j = 6; j < 18; j++)
    {
        checksum += btemp[j];
    }

    btemp[18] = checksum;
    memcpy(res, btemp, 20);
    len = 20;
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
        glog::GetInstance()->AddLine("�ϰ�ԭ���ݿ�");
    }

    return TRUE;
}
/*
*  comaddr ���ص�ַ
*  C       ������
*  AFN  ������
*  SEQ  ������
*  DA
*  DT
*  v_B  ��������
* des  ���յ�������
*/
int CIOCP::buidByte(string comaddr, BYTE C, BYTE AFN, BYTE SEQ, SHORT DA, SHORT DT, vector<BYTE>&v_b, BYTE des[])
{
    BYTE addrArea[4] = {0};

    if(comaddr.size() != 8)
    {
        PostLog("ͨ�ŵ�ַ���ϸ�ʽ");
        return 0;
    }

    int n =  hex2str(comaddr, addrArea);

    if(n > 0)
    {
        BYTE hexData[256] = {0};
        hexData[0] = 0x68;
        hexData[5] = 0x68;
        hexData[6] = C;//0x4;   //������  �������ǴӶ�  ���л������� 0x4  0000 0100
        hexData[7] = addrArea[1]; //parseInt(sprintf("0x%02d", addrArea[1]), 16)             //��ַ��
        hexData[8] = addrArea[0];              //parseInt(sprintf("0x%02d", addrArea[0]), 16)   //��ַ��
        hexData[9] = addrArea[3];                           //parseInt(sprintf("0x%02d", addrArea[3]), 16)
        hexData[10] = addrArea[2];         //parseInt(sprintf("0x%02d", addrArea[2]), 16)
        hexData[11] = 0x02;  //��ַC  ����ַ�����ַ
        hexData[12] = AFN;  //������
        hexData[13] = SEQ;  //֡����
        hexData[15] = DA >> 8 & 0x00ff; //   DA1
        hexData[14] = DA & 0x00ff;
        hexData[17] = DT >> 8 & 0x00ff;
        hexData[16] = DT & 0x00ff;

        for(int i = 0; i < v_b.size(); i++)
        {
            hexData[18 + i] = v_b[i];
        }

        int len1 = 18 - 6 + v_b.size();   //18�ǹ̶�����  6����ͷ
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
    PostLog("�пͻ������� ͨ��ָ��:%p  �ͻ�������:%d", lp_io, lp_io->fromtype);
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
    PostLog("ExitSocket  lp_io:%p  List1 count:%d List2 count:%d �ͻ�������:%d", lp_io, n11, n00, lp_io->fromtype);
    //EnterCriticalSection(&crtc_sec);
    //if(lp_io->fromtype == SOCKET_FROM_GAYWAY)
    //  {
    //    //�������ͻ�������
    //    string comaddr = lp_io->gayway;
    //    setOnline(comaddr, 0);
    //    //�Ƴ�������
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
    //��Ϣ����ɾ��   ��Ϣ���д������ҳ�ͻ���
    //  EnterCriticalSection(&crtc_sec);
    DeleteByIo((ULONG_PTR)lp_io->pUserData);
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

    if(dwBytes == 15)
    {
        char vv[16] = {0};
        memcpy(vv, lp_io->buf, dwBytes);

        if(_stricmp(vv, "www.cdebyte.com") == 0)
        {
            string address = "17020101";
            lp_io->fromtype = SOCKET_FROM_GAYWAY;
            map<string, IOCP_IO_PTR>::iterator it = m_mcontralcenter.find(address);

            if(it == m_mcontralcenter.end())
            {
                m_mcontralcenter.insert(pair<string, IOCP_IO_PTR>(address, lp_io));
            }
            else
            {
                it->second = lp_io;
            }

            memset(lp_io->buf, 0, BUFFER_SIZE);
            unsigned char hexData[6] =
            {
                0x61, 0x62, 0x63, 0x64, 0x65, 0x66
            };
            memcpy(lp_io->buf, hexData, sizeof(hexData));
            lp_io->wsaBuf.buf = lp_io->buf;
            lp_io->wsaBuf.len = sizeof(hexData);
            lp_io->operation = IOCP_WRITE;
            DataAction(lp_io, lp_key);
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
            PostLog("web �ϰ���β:lp_io:%p ����:%d", lp_io, dwBytes);

            if(alllenth > BUFFER_SIZE)
            {
                PostLog("�����ȹ���:%d", alllenth);

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
                //websocket�ϰ�����
                pBREAKPCK webpack = new BREAK_PACK;
                BYTE *b1 = new BYTE[datalen];
                memset(b1, 0, datalen);
                memcpy(b1, src, datalen);
                webpack->b = b1;
                webpack->len = datalen;
                m_pack.insert(make_pair(lp_io, webpack));
                PostLog("web �ϰ���ͷ:lp_io:%p ����:%d", lp_io, datalen);
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
            PostLog("web���˳� ͨ��ָ��:%p", lp_io);
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

    //FIN:1λ������������Ϣ�Ƿ���������Ϊ1�����ϢΪ��Ϣβ��,���Ϊ�����к������ݰ�;
    if(Fin != 1)
    {
        return ret;
    }

    // �����չλ������
    if(RSV1 == 1 || RSV2 == 1 || RSV3 == 1)
    {
        return ret;
    }

    // maskλ, Ϊ1��ʾ���ݱ�����
    if(Mask != 1)
    {
        return ret;
    }

    BYTE opcode = msg[0] & 0x0f;
    // ������
    uint16_t payloadLength = 0;
    uint8_t payloadFieldExtraBytes = 0;

    if(opcode == WS_TEXT_FRAME)
    {
        // ����utf-8������ı�֡
        payloadLength = static_cast<uint16_t >(msg[1] & 0x7f);

        if(payloadLength == 0x7e)   //0111 1110     //126 7e  �������ֽ��ǳ��� :  127  7f �������ֽ��ǳ���
        {
            uint16_t payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &msg[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }
        else if(payloadLength == 0x7f)
        {
            // ���ݹ���,�ݲ�֧��
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
