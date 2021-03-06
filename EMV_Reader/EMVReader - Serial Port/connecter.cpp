#include "connecter.h"

BYTE ResetCommand[] = {
    0x3B, 0x8E, 0x80, 0x01, 0x80, 0x31, 0x80, 0x66,
    0xB0, 0x84, 0x0C, 0x01, 0x6E, 0x01, 0x83, 0x00,
    0x90, 0x00, 0x1D
};

BYTE QueryCommand[] = {
    0x00, 0xA4, 0x04, 0x00, 0x0E, 0x31, 0x50, 0x41,
    0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44,
    0x46, 0x30, 0x31, 0x00
};

BYTE InitCommand[] = {
    0x00, 0xA4, 0x04, 0x00, 0x08, 0xA0, 0x00, 0x00,
    0x03, 0x33, 0x01, 0x01, 0x03, 0x00
};

BYTE BeforeAid[] = {
    0x00, 0xB2, 0x01, 0x0C, 0x00
};

BYTE AidCommand[] = {
    0x00, 0xB2, 0x02, 0x0C, 0x00
};

BYTE RootCommand[] = {
    0x00, 0xA4, 0x04, 0x00, 0x08, 0xA0, 0x00, 0x00,
    0x03, 0x33, 0x01, 0x01, 0x01, 0x00
};

BYTE UnKown_1[] = {
    0x80, 0xA8, 0x00, 0x00, 0x23, 0x83, 0x21, 0x30, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x09, 0x78, 0x14, 0x12, 0x08, 0x00, 0xDE, 0xAD, 0xBE, 0xEF,
    0x00
};

BYTE UnKown_2[] = {
    0x80, 0xCA, 0x9F, 0x36, 0x00
};

BYTE UnKown_3[] = {
    0x80, 0xCA, 0x9F, 0x13, 0x00
};

BYTE UnKown_4[] = {
    0x80, 0xCA, 0x9F, 0x17, 0x00
};

BYTE UnKown_5[] = {
    0x80, 0xCA, 0x9F, 0x4D, 0x00
};

BYTE UnKown_6[] = {
    0x80, 0xCA, 0x9F, 0x4F, 0x00
};

Connecter::Connecter()
{
    ClearReadStatus();
}

void Connecter::ClearReadStatus()
{
    guy_name = "";
    guy_name_ex = "";
    id_num = "";
    id_type = "";
    card_name = "";
    begin_time = "";
    end_time = "";
    card_type = "";
    card_num = "";

    std::list<CUSTUMRECORD>::iterator it;
    it = RecordList.begin();
    while (it != RecordList.end()) {
        it = RecordList.erase(it);
    }
}

void Connecter::SetHSC(SCARDCONTEXT hSCOrigin)
{
    hSC = hSCOrigin;
}

int Connecter::TestConnect()
{
    if(ReaderName.isEmpty())
        return false;

    DWORD dwAP;
    LONG  lReturn;

    lReturn = SCardConnect(hSC,
        ReaderName.toStdWString().c_str(),
        SCARD_SHARE_SHARED,
        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
        &hCardHandle,
        &dwAP);

    return lReturn;
}

// Must call TestConnect() before this fuction!
// Or software will be crushed!
int Connecter::StartReadInfo()
{
    BYTE* volatile pbRecv = new BYTE[RECV_BUFFER_SIZE];
    DWORD dwRecv = RECV_BUFFER_SIZE;

    int lReturn;

    ClearReadStatus();

    // RESET
    lReturn = SCardTransmit(hCardHandle,
                            SCARD_PCI_T1,
                            ResetCommand,
                            sizeof(ResetCommand),
                            NULL, pbRecv, &dwRecv);

    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }


    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);

    //Sleep(50);

    // Query
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, QueryCommand, sizeof(QueryCommand), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);

    //Sleep(50);

    // BeforeAid
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, BeforeAid, sizeof(BeforeAid), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }
    for (int i = 0; i < 8; i++) {
        *(InitCommand + 5 + i) = *(pbRecv + 6 + i);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);

    //Sleep(50);

    // Aid
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, AidCommand, sizeof(AidCommand), NULL, pbRecv, &dwRecv);

    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);

    //Sleep(50);


    // RESET
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, ResetCommand, sizeof(ResetCommand), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);

    //Sleep(50);


    // Init 这里需要把上一步的结果拖下来
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, InitCommand, sizeof(InitCommand), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);\
        PrintTLVList(TLVList);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);

    //Sleep(50);

    // Unkown_1
    //  0x14, 0x12, 0x08, 这几个字节是当天日期
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, UnKown_1, sizeof(UnKown_1), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);

    //Sleep(50);

    // Unkown_2
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, UnKown_2, sizeof(UnKown_2), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);

    //Sleep(50);

    // Unkown_3
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, UnKown_3, sizeof(UnKown_3), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }

    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);

    //Sleep(50);

    // Unkown_4
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, UnKown_4, sizeof(UnKown_4), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);


    //Sleep(50);

    // Unkown_5
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, UnKown_5, sizeof(UnKown_5), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);


    //Sleep(50);

    // Unkown_6
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, UnKown_6, sizeof(UnKown_6), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        TLVPackage::Connector(pbRecv, dwRecv, TLVList);
        PrintTLVList(TLVList);
    }
    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);


    //Sleep(50);
    QTime time;
    time.start();
    // 测试交易记录是否在00 B2 01 5C 00 位置
    BYTE  TestCommand[5] = { 0x00, 0xB2, 0x01, 0x5C, 0x00 };
    lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, TestCommand, sizeof(TestCommand), NULL, pbRecv, &dwRecv);
    if(pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        dwRecv = RECV_BUFFER_SIZE;
        ZeroMemory(pbRecv, dwRecv);
        for (char sfi = 1; sfi <= 3; sfi++) {
            for (char rec = 1; rec <= 16; rec++) {
                    SendApdu(
                        0x00,
                        0xB2,
                        rec,
                        (sfi << 3) | 4,
                        0x00);
            }
        }
        for (char rec = 0x01; rec <= 0x0A; rec++) {
                SendApdu(
                    0x00,
                    0xB2,
                    rec,
                    0x5C,
                    0x00);
        }
    } else {
        for (char sfi = 1; sfi <= 31; sfi++) {
            for (char rec = 1; rec <= 16; rec++) {
                    SendApdu(
                        0x00,
                        0xB2,
                        rec,
                        (sfi << 3) | 4,
                        0x00);
            }
        }
    }
    qDebug()<<"SendApdu Time: " << time.elapsed()/1000.0 << "s";

    delete []pbRecv;

    SCardDisconnect(hCardHandle, SCARD_RESET_CARD);
    return TRUE;
}

int Connecter::SendApdu(BYTE byte1, BYTE byte2, BYTE byte3, BYTE byte4, BYTE byte5)
{
    BYTE  pbRecv[RECV_BUFFER_SIZE] = { 0x00 };
    DWORD dwRecv = RECV_BUFFER_SIZE;
    BYTE  Command[5] = { 0x00 };
    //TLVEntity tlvEntity[100];
    //unsigned int tlv_count;

    Command[0] = byte1;
    Command[1] = byte2;
    Command[2] = byte3;
    Command[3] = byte4;
    Command[4] = byte5;

    int lReturn = SCardTransmit(hCardHandle, SCARD_PCI_T1, Command, sizeof(Command), NULL, pbRecv, &dwRecv);
    if (pbRecv[dwRecv - 2] == 0x90 && pbRecv[dwRecv - 1] == 0x00) {
        if (pbRecv[0] == 0x6F ||
            pbRecv[0] == 0x70 ||
            pbRecv[0] == 0x77 ||
            pbRecv[0] == 0x9F) {
            TLVPackage::Connector(pbRecv, dwRecv, TLVList);
            PrintTLVList(TLVList);
        }
        else {
            //QTime time;
            //time.start();
            PrintConsumerRecord(pbRecv);
            //qDebug()<<time.elapsed()/1000.0<<"s";
        }
    }

    dwRecv = RECV_BUFFER_SIZE;
    ZeroMemory(pbRecv, dwRecv);
    return lReturn;
}

void Connecter::PrintTLVList(
    std::list<TLVPackage *> &TLVList)
{
    std::list<TLVPackage *>::iterator it;
    it = TLVList.begin();
    while (it != TLVList.end()) {
        TLVPackage *temp = TLVList.front();
        PrintTLVInfo(temp);
        delete temp;
        temp = NULL;
        it = TLVList.erase(it);
    }
}

void Connecter::PrintTLVInfo(
    const TLVPackage* CurrentPackage)
{
    if (CurrentPackage->tagSize == 2 ) {
        if ((CurrentPackage->tag[0] == 0x5F && CurrentPackage->tag[1] == 0x20) ||
            (CurrentPackage->tag[0] == 0x9F && CurrentPackage->tag[1] == 0x61) ||
            (CurrentPackage->tag[0] == 0x9F && CurrentPackage->tag[1] == 0x62) ||
            (CurrentPackage->tag[0] == 0x5F && CurrentPackage->tag[1] == 0x2D) ||
            (CurrentPackage->tag[0] == 0x5F && CurrentPackage->tag[1] == 0x25) ||
            (CurrentPackage->tag[0] == 0x5F && CurrentPackage->tag[1] == 0x24) ||
            (CurrentPackage->tag[0] == 0x9F && CurrentPackage->tag[1] == 0x12) ||
            (CurrentPackage->tag[0] == 0x9F && CurrentPackage->tag[1] == 0x0B)) {
            goto __print;
        }
        else {
            return;
        }
    }
    else if (CurrentPackage->tagSize == 1) {
        if (CurrentPackage->tag[0] == 0x50 ||
            CurrentPackage->tag[0] == 0x5A) {
            goto __print;
        }
        else
            return;
    }
    else
        return;
__print:

    if (CurrentPackage->tagSize == 2) {
            // 持卡人姓名
            if (CurrentPackage->tag[0] == 0x5F && CurrentPackage->tag[1] == 0x20) {
                if (!guy_name.isEmpty())
                    return;
                //guy_name += QString::fromLocal8Bit(std::string("持卡人姓名： ").c_str());
                guy_name += QString::fromLocal8Bit(std::string(CurrentPackage->value.begin(), CurrentPackage->value.end()).c_str());
                guy_name = guy_name.trimmed();
                //guy_name += QString::fromLocal8Bit("\n");
            }
            else if (CurrentPackage->tag[0] == 0x9F && CurrentPackage->tag[1] == 0x0B) {
                if (!guy_name_ex.isEmpty())
                    return;
                //guy_name_ex += QString::fromLocal8Bit(std::string("持卡人姓名扩展： ").c_str());
                guy_name_ex += QString::fromLocal8Bit(std::string(CurrentPackage->value.begin(), CurrentPackage->value.end()).c_str());
                guy_name_ex = guy_name_ex.trimmed();
                //guy_name_ex += QString::fromLocal8Bit("\n");
            }
            // 持卡人证件号
            else if (CurrentPackage->tag[0] == 0x9F && CurrentPackage->tag[1] == 0x61) {
                if (!id_num.isEmpty())
                    return;
                //id_num += QString::fromLocal8Bit(std::string("持卡人证件号： ").c_str());
                id_num += QString::fromLocal8Bit(std::string(CurrentPackage->value.begin(), CurrentPackage->value.end()).c_str());
                id_num = id_num.trimmed();
                //id_num += QString::fromLocal8Bit("\n");
            }
            // 持卡人证件类型
            else if (CurrentPackage->tag[0] == 0x9F && CurrentPackage->tag[1] == 0x62) {
                if (!id_type.isEmpty())
                    return;
                id_type += QString::fromLocal8Bit(std::string("持卡人证件类型：").c_str());
                if (CurrentPackage->value[0] == 0x00) {
                    id_type += QString::fromLocal8Bit(std::string("身份证").c_str());
                    //id_type += QString::fromLocal8Bit("\n");
                }
                else {
                    id_type += QString::fromLocal8Bit(std::string("未知").c_str());
                    //id_type += QString::fromLocal8Bit("\n");
                }
            }
            // 卡名
            else if (CurrentPackage->tag[0] == 0x9F && CurrentPackage->tag[1] == 0x12) {
                if (!card_name.isEmpty())
                    return;
                //card_name += QString::fromLocal8Bit(std::string("发卡名： ").c_str());
                card_name += QString::fromLocal8Bit(std::string(CurrentPackage->value.begin(), CurrentPackage->value.end()).c_str());
                card_name = card_name.trimmed();
                //card_name += QString::fromLocal8Bit("\n");
            }
            // 发卡时间
            else if (CurrentPackage->tag[0] == 0x5F && CurrentPackage->tag[1] == 0x25) {
                if (!begin_time.isEmpty())
                    return;
                QString temp;
                //begin_time += QString::fromLocal8Bit(std::string("发卡时间： ").c_str());
                begin_time += QString::fromLocal8Bit(std::string("20").c_str());
                temp.sprintf("%02X", CurrentPackage->value[0]);
                begin_time += temp;
                begin_time += QString::fromLocal8Bit(std::string(".").c_str());
                temp.sprintf("%02X", CurrentPackage->value[1]);
                begin_time += temp;
                begin_time += QString::fromLocal8Bit(std::string(".").c_str());
                temp.sprintf("%02X", CurrentPackage->value[2]);
                begin_time += temp;
                //begin_time += QString::fromLocal8Bit(std::string(".").c_str());
            }
            // 卡过期时间
            else if (CurrentPackage->tag[0] == 0x5F && CurrentPackage->tag[1] == 0x24) {
                if (!end_time.isEmpty())
                    return;
                QString temp;
                //end_time += QString::fromLocal8Bit(std::string("过期时间： ").c_str());
                end_time += QString::fromLocal8Bit(std::string("20").c_str());
                temp.sprintf("%02X", CurrentPackage->value[0]);
                end_time += temp;
                end_time += QString::fromLocal8Bit(std::string(".").c_str());
                temp.sprintf("%02X", CurrentPackage->value[1]);
                end_time += temp;
                end_time += QString::fromLocal8Bit(std::string(".").c_str());
                temp.sprintf("%02X", CurrentPackage->value[2]);
                end_time += temp;
                //end_time += QString::fromLocal8Bit(std::string(".").c_str());
            }
            else {
                return;
            }
        }
        else if (CurrentPackage->tagSize == 1) {
            // 持卡类型
            if (CurrentPackage->tag[0] == 0x50) {
                if (!card_type.isEmpty())
                    return;
                //card_type += QString::fromLocal8Bit(std::string("持卡类型：").c_str());
                card_type += QString::fromLocal8Bit(std::string(CurrentPackage->value.begin(), CurrentPackage->value.end()).c_str());
                //card_type += QString::fromLocal8Bit("\n");
            }
            // 卡号
            else if (CurrentPackage->tag[0] == 0x5A) {
                if (!card_num.isEmpty())
                    return;
                QString temp;
                //card_num += QString::fromLocal8Bit(std::string("卡号： ").c_str());
                for (unsigned int i = 0; i < CurrentPackage->valueSize; i++){
                    temp.sprintf("%02X", CurrentPackage->value[i]);
                    card_num += temp;
                }
                //card_num += QString::fromLocal8Bit("\n");
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
}


void Connecter::PrintConsumerRecord(BYTE * buffer)
{
    CUSTUMRECORD Record;

    // 打印交易时间
    Record.Time = "";
    //Record.Time += QString::fromLocal8Bit("交易时间： ");
    QString temp;
    temp.sprintf("20%02X", buffer[0]);
    temp += QString::fromLocal8Bit("年");
    Record.Time += temp;
    temp.sprintf("%02X", buffer[1]);
    temp += QString::fromLocal8Bit("月");
    Record.Time += temp;
    temp.sprintf("%02X", buffer[2]);
    temp += QString::fromLocal8Bit("日");
    Record.Time += temp;

    temp = "";
    temp.sprintf(" %02X:%02X:%02X",
        buffer[3], buffer[4], buffer[5]);
    Record.Time += temp;

    // 打印交易金额
    Record.Money = "";
    //Record.Money += QString::fromLocal8Bit("交易金额： ￥");
    Record.Money += QString::fromLocal8Bit("￥");
    BYTE  money[5] = {0};
    BYTE * current = money;

    for (int i = 0; i < 5; i++)
        *(money + i) = *(buffer + 6 + i);
    while (*current == 0)
        current++;

    if (money[0] == 0 && money[1] == 0 && money[2] == 0 && money[3] == 0 && money[4] == 0) {
    //if (current - money == 5) {
        Record.Money += "0";
    }
    else {
        temp = "";
        temp.sprintf("%X", current[0]);
        Record.Money += temp;
        for (current += 1; current - money < 5; current++) {
            temp.sprintf("%02X", current[0]);
            Record.Money += temp;
        }
    }
    temp = "";
    temp.sprintf(".%02X", buffer[12]);
    Record.Money += temp;

    // 打印交易地点
    char * place = new char[20];
    ZeroMemory(place, 20);
    for (int i = 0; i < 20; i++)
        *(place + i) = *(buffer + 22 + i);

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    temp.clear();
    temp = codec->toUnicode(place);
    Record.Position.clear();
    //Record.Position += QString::fromLocal8Bit("交易地点： ");
    //temp = QString::fromLocal8Bit(place);
    Record.Position += temp;
    int iLength = Record.Position.toLocal8Bit().length();
    if(iLength >= 16) {
        for (int i = 0; i < 19; i++)
            *(place + i) = *(buffer + 22 + i);
        place[19] = 0x00;
        Record.Position = QString::fromLocal8Bit(place);;
    }

    // 打印交易类型
    Record.Type = "";
    if (buffer[42] == 0x01) {
        Record.Type += QString::fromLocal8Bit("提款/现金付款");
    }
    else if (buffer[42] == 0x30) {
        Record.Type += QString::fromLocal8Bit("查询可用现金");
    }
    else {
        Record.Type += QString::fromLocal8Bit("POS机交易");
    }

    RecordList.push_back(Record);
    delete []place;
}

