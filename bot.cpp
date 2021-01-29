#include "bot.h"
#include <QDate>

//ipLogin portLogin clientxhash clienthash version ndc mdp serveur canal camp wHookAdd langCode
// 79.110.84.75 4002 26179febd6c3e13b571c920e4157b966 5a45a96bf0478a5c82fabdd9b162d591 0.9.3.3127 username password 1 1 D 40

Bot::Bot(int argc, char *argv[])
{
    Q_UNUSED(argc)
    qsrand(static_cast<uint>(QTime::currentTime().msecsSinceStartOfDay()));
    date = QDateTime::currentDateTime().toOffsetFromUtc(QDateTime::currentDateTime().offsetFromUtc()).toString("dd/MM/yyyy h:mm:ss");
    ipLogin = argv[1];
    portLogin = static_cast<unsigned short>(atoi(argv[2]));
    clientxhash = argv[3];
    clienthash = argv[4];
    version = argv[5];
    username = argv[6];
    password = argv[7];
    serveur = argv[8];
    canal = argv[9];
    camp = QString(argv[10]).toUpper();
    timeMinute = QString(argv[11]).toInt() * 60;
    wHookAdd = argv[12];
    langcode = argv[13];

    code = GetRandomString(qrand()%2 + 3);
    // Please, change this shit, you have to generate a random uuid v4
    QString debut = "6c49bda5-bf1c-4713-b2e0-123412341234";
    gfuid = debut;//+GetRandomString(12);
    qInfo() << "Login : " << ipLogin << ":" << portLogin << " Server : " << serveur;
    qInfo() << "NostaleClientX.exe hash : " << clientxhash;
    qInfo() << "NostaleClient.exe hash  : " << clienthash;
    qInfo() << "Credentials : " << username << " | " << password;
    qInfo() << "=========================" << date << "=========================";
    Initialise();
    socket->connectToHost(ipLogin, portLogin);
}

void Bot::connected()
{
    if(loginServer)
        socket->write(crypt->createLoginPacket(username,password, langcode));
    else
    {
        QString temp = QString::number(getPid())+" "+QString::number(sid);
        QByteArray temp2 = crypt->encryptGamePacket(temp,sid,true);
        socket->write(temp2);
        QTimer::singleShot(1000, this, [this]()
        {
            send(username + " GF " + langcode);
            send("thisisgfmode");
        });
        qInfo() << "Game server connection OK";
    }
}

void Bot::disconnected()
{
    if(loggedin&&loginServer)
    {
        loginServer = false;
        qInfo() << "Disconnected from Login Server";
    }
    else
    {
        qInfo() << "Disconnected from Game Server";
        if(OKSent)
        {
            socket->connectToHost(gameip, 4019);
            QTimer::singleShot(1000, [this]{

                send("c_close 0");
                send("f_stash_end");
                send("c_close 1");
                send(QString::number(sid));
                QTimer::singleShot(1000, [this]{
                    send("DAC " + username + " 0");
                    send("glist 0 0");
                });
            });
        }
    }
}

void Bot::recievedPacket()
{
    QStringList packets = decrypt(socket->readAll());
    foreach (QString packet, packets)
    {
        QStringList packs;
        packs = packet.replace("\n","").split(" ", Qt::SkipEmptyParts);
        if(loginServer)
        {
            if(packs[0]=="NsTeST")
            {
                username = packs[2];
                loggedin = true;
                socket->close();
                QString packetList = "";
                for(int i=0;i<packets.size();i++)
                    packetList = packetList + packets[i];
                manageServChann(packetList);
            }
            if(packs[0] == "failc")
            {
                loggedin = false;
                if(packs[1] == "1")
                    qInfo() << "Mauvaise version.";
                if(packs[1] == "2" || packs[1] == "6" || packs[1] == "8")
                    qInfo() << "Erreur Login inconnue.";
                if(packs[1] == "3")
                    qInfo() << "Serveur en maintenance.";
                if(packs[1] == "4")
                    qInfo() << "Compte deja connecte.";
                if(packs[1] == "5" || packs[1] == "9")
                    qInfo() << "Mauvaise identifiants.";
                if(packs[1] == "7")
                    qInfo() << "Compte banni.";
            }
        }else{
                if(packs[0] == "clist_end")
                {
                    send("c_close 0");
                    send("f_stash_end ");
                    send("c_close 1");
                    QTimer::singleShot(1500 + qrand() % 2000, [this]{send("select 0");});
                }
            if(packs[0]=="OK"){
                OKSent = true;
                send("game_start");
                send("lbs 0");
                keepAlive->start(60000);
            }
            if(packs[0]=="c_info" && passed == false)
            {
                passed = true;
                send("c_close 1");
                userid = packs[6];
                pseudonym = packs[1];
                qDebug() << "Pseudonym : " << packs[1];
            }
            if(packs[0] == "sc_p_stc")
            {
                send("npinfo 0");
            }
            if(packs[0] == "mapout")
            {
                send("c_close 1");
                send("c_close 0");
                send("f_stash_end");
                send("c_close 1");
                send("c_close 1");
            }
            if(packs[0] == "fc")
            {
                packetToHandle = packet;
                timerFcPacket->start(1000);
                qDebug() << "Rcv: " << packetToHandle;
            }
            if(packs[0] == "spk")
            {
                if(packs.size() >= 5)
                {
                    // Was used for my familly in order to swap between angel/demon, to change time before @everyone-ing, or to resend a webhook request
                    // Whisper the character with the code and the arguments you want :
                    // "/pseudonym code" => Resend a Webhook request
                    // "/pseudonym code A" => switch to angel
                    // "/pseudonym code D" => switch to demon
                    // "/pseudonym code 40" => a webhook will be sent 40 minutes before the raid ends (10 minutes before it opens)
                    packs.erase(packs.begin() + packs.size() - 2, packs.begin() + packs.size());
                    if(packs[5] == code)
                    {
                        if(packs.size() == 6)
                        {
                            mustBeTreated = true;
                            code = GetRandomString(qrand()%2 + 3);
                            forceRefresh = true;
                            handleFcPacket();
                        }
                        if(packs.size() == 7)
                        {
                            if(packs[6].toUpper().startsWith("D"))
                            {
                                camp = "D";
                            }
                            if(packs[6].toUpper().startsWith("A"))
                            {
                                camp = "A";
                            }
                            if(packs[6].toInt() != 0)
                            {
                                timeMinute = packs[6].toInt();
                            }
                        }
                    }
                }
            }
        }
    }
}

void Bot::sendPulsePacket()
{
    runtime++;
    send("pulse "+QString::number(runtime*60)+" 0");
}

void Bot::Initialise()
{
    crypt = new Cryptography(this);
    crypt->setGFuid(gfuid);
    crypt->setClienthash(clienthash);
    crypt->setClientXhash(clientxhash);
    crypt->setVersion(version);
    socket = new QTcpSocket(this);
    keepAlive = new QTimer(this);
    timerFcPacket = new QTimer(this);
    wHook = nullptr;
    sid = 0;
    pid = -1;
    gameport = 0;
    runtime = 0;
    gameip = "";
    loginServer = true;
    loggedin = false;
    entry = false;
    passed = false;
    OKSent = false;
    currentTime = 0;
    mustBeTreated = true;
    lastAction = "0";
    forceRefresh = false;

    connect(socket, &QTcpSocket::readyRead, this, &Bot::recievedPacket);
    connect(socket, &QTcpSocket::connected, this, &Bot::connected);
    connect(socket, &QTcpSocket::disconnected, this, &Bot::disconnected);

    connect(keepAlive, &QTimer::timeout, this, &Bot::sendPulsePacket);
    connect(timerFcPacket, &QTimer::timeout, this, &Bot::handleFcPacket);
}

void Bot::send(QString packet, bool loginServer)
{
    Q_UNUSED(loginServer)
    QByteArray msg = encrypt(QString::number(getPid())+" "+packet);
    socket->write(msg);
}

int Bot::getPid()
{
    if(pid == -1)
        pid = 23346;
    if(pid == 65534)
        pid = -1;
    pid++;
    return pid;
}

void Bot::manageServChann(QString packetList)
{
    QStringList packetLine = packetList.replace("\n"," ").split(" ", Qt::SkipEmptyParts);
    packetLine.removeFirst();
    packetLine.removeFirst();
    packetLine.removeFirst();
    packetLine.removeFirst();

    while(packetLine.size() >= 0 && packetLine[0].size() < 17)
    {
        sid = packetLine[0].toInt();
        packetLine.removeFirst();
    }

    QStringList packetDot;
    for(int i = 0 ; i < packetLine.size() -1 ; i++)
    {
        packetDot << packetLine[i].split(".");
    }
    for(int i = 5 ; i < packetDot.size() ; i = i+6)
    {
        if(packetDot[i-1] == canal && packetDot[i-2].at(packetDot[i-2].size()-1) == serveur)
        {
            QStringList delimiter = packetDot[i-2].split(":");
            gameip = packetDot[i-5] + "." + packetDot[i-4] + "." + packetDot[i-3] + "." + delimiter[0];
            gameport = delimiter[1].toUShort();
            //qDebug() << gameip << gameport << ip << port;
            QTimer::singleShot(1000, [this](){socket->connectToHost(gameip, gameport);});
        }
    }
}

QStringList Bot::decrypt(QByteArray msg)
{
    if(loginServer)
    {
        QString temp;
        temp = crypt->decryptLoginPacket(msg, static_cast<std::size_t>(msg.length()));
        return {temp};
    }
    else
        return crypt->decryptGamePacket(msg, static_cast<std::size_t>(msg.length()));
}

QByteArray Bot::encrypt(QString packet, bool loginserver)
{
    Q_UNUSED(loginserver)
    if(!loginServer)
        return crypt->encryptGamePacket(packet,sid,false);
    else
        return "";
}

QString Bot::GetRandomString(int length) const
{
    const QString possibleCharacters("abcdefghijklmnopqrstuvwxyz0123456789");
    QString randomString;
    for(int i = 0; i < length; ++i)
    {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}

//fc  [Camp 1:Angel 2:Demon] [lefftTime] [%Angel] [0: nothing 1:MukrajuPop 2:?? 3:RaidPop] [CurrentTime] [MaxTime] [raidMorcos] [raidHatus] [raidCalvi]
//[raidBerios] 0 [%Demon] [0:nothing 1:MukrajuPop 2: ??? 3:RaidPop] [CurrentTime] [MaxTime] [raidMorcos] [raidHatus] [raidCalvi] [raidBerios] 0
void Bot::handleFcPacket()
{
    bool isRaidAnge = false;
    bool isRaidDemon = false;
    bool isMukraAnge = false;
    bool isMukraDemon = false;

    int percentAnge = 0;
    int percentDemon = 0;
    int maxTime = 0;

    QString day = QDateTime::currentDateTime().toOffsetFromUtc(QDateTime::currentDateTime().offsetFromUtc()).toString("dd/MM/yyyy");
    QString hour = QDateTime::currentDateTime().toOffsetFromUtc(QDateTime::currentDateTime().offsetFromUtc()).toString("hh:mm:ss");
    QString valueField1 = "";
    QString valueField2 = "";
    QString thumbnail = "";
    QString icone = "";
    QString footerDate = "Refresh: " + day + " at " + hour;
    QString color = "";

    QStringList packs = packetToHandle.split(" ");

    percentAnge = packs[3].toInt();
    percentDemon = packs[12].toInt();
    isRaidAnge = (packs[4] == "3" ? true : false);
    isMukraAnge = ((packs[4] == "1" || packs[4] == "2") ? true : false);
    isRaidDemon = (packs[13] == "3" ? true : false);
    isMukraDemon = ((packs[13] == "1" || packs[13] == "2") ? true : false);
    if(currentTime == timeMinute) forceRefresh = true;
    if(lastAction.startsWith("M") && (isRaidAnge || isRaidDemon))
    {
        currentTime = 0;
    }
    if(currentTime == 0)
        currentTime = packs[5].toInt();
    maxTime = packs[6].toInt();
    if(maxTime == 0)
    {
        if(currentTime == 0)
            currentTime = packs[14].toInt();
        maxTime = packs[15].toInt();
    }
    if(lastAction != "0" && (!isRaidAnge && !isRaidDemon && !isMukraAnge && !isMukraDemon))
    {
        mustBeTreated = true;
        lastAction = "0";
    }
    if(percentAnge > percentDemon)
    {
        valueField1 = "__" + QString::number(percentAnge) + "%__";
        valueField2 = QString::number(percentDemon) + "%";
        color = AVANTAGEANGE;
        icone = ICONE_NO;
        thumbnail = THUMBNAIL_AVANGE;
    }
    else if(percentDemon > percentAnge)
    {
        valueField1 = QString::number(percentAnge) + "%";
        valueField2 = "__" + QString::number(percentDemon) + "%__";
        color = AVANTAGEDEMON;
        icone = ICONE_NO;
        thumbnail = THUMBNAIL_AVDEMON;
    }
    else
    {
        valueField1 = QString::number(percentAnge) + "%";
        valueField2 = QString::number(percentDemon) + "%";
        color = EXAEQUO;
        icone = ICONE_NO;
        thumbnail = THUMBNAIL_EXAEQUO;
    }
    if(isMukraAnge && lastAction != "MA")
    {
        valueField1 = "Angel Mukraju.";
        color = MUKRAANGE;
        mustBeTreated = true;
        lastAction = "MA";
        valueField2 = QString::number(percentDemon) + "%";
        icone = ICONE_MUKRA;
        thumbnail = THUMBNAIL_MUKRAANGE;
    }
    else if(isMukraDemon && lastAction != "MD")
    {
        valueField2 = "Demon Mukraju.";
        color = MUKRADEMON;
        mustBeTreated = true;
        lastAction = "MD";
        valueField1 = QString::number(percentAnge) + "%";
        icone = ICONE_MUKRA;
        thumbnail = THUMBNAIL_MUKRADEMON;
    }
    else if(isRaidAnge)
    {
        int min = currentTime/60;
        int s = currentTime%60;
        if(currentTime <= maxTime/2 && (lastAction != "RAO" || forceRefresh))
        {
            valueField1 = "Angel raid.\\nRemaining time : " + QString::number(min) + "min" + QString::number(s) + "s/" + QString::number(maxTime/60) + "min.\\nRaid opened.";
            lastAction = "RAO";
            mustBeTreated = true;
        }
        else if(currentTime > maxTime/2 && (lastAction != "RAF" || forceRefresh))
        {
            valueField1 = "Angel raid.\\nRemaining time : " + QString::number(min) + "min" + QString::number(s) + "s/" + QString::number(maxTime/60) + "min.\\nRaid closed.";
            lastAction = "RAF";
            mustBeTreated = true;
        }
        color = RAIDANGE;

        thumbnail = THUMBNAIL_RA;
        icone = ICONE_RA;
        valueField2 = QString::number(percentDemon) + "%";
    }
    else if(isRaidDemon)
    {
        int min = currentTime/60;
        int s = currentTime%60;
        if(currentTime <= maxTime/2 && (lastAction != "RDO" || forceRefresh))
        {
            valueField2 = "Demon raid.\\nRemaining time : " + QString::number(min) + "min" + QString::number(s) + "s/" + QString::number(maxTime/60) + "min.\\nRaid opened.";
            lastAction = "RDO";
            mustBeTreated = true;
        }
        else if(currentTime > maxTime/2 && (lastAction != "RDF" || forceRefresh))
        {
            valueField2 = "Demon raid.\\nRemaining time : " + QString::number(min) + "min" + QString::number(s) + "s/" + QString::number(maxTime/60) + "min.\\nRaid closed.";
            lastAction = "RDF";
            mustBeTreated = true;
        }
        color = RAIDDEMON;
        thumbnail = THUMBNAIL_RD;
        icone = ICONE_RD;
        valueField1 = QString::number(percentAnge) + "%";
    }
    QByteArray json =  "{"
                       "\"username\": \"Ayugract4\","
                       "\"avatar_url\": \"https://cdn.discordapp.com/attachments/607377140387807332/607377164605849631/SP3MFille.png\","


                       "\"content\": \"" +
            ((((lastAction == "RDO" || lastAction == "RDF") && camp == "D")
            ||((lastAction == "RAO" || lastAction == "RAF") && camp == "A"))
            && (currentTime <= timeMinute && currentTime != 0) ?
                 QByteArray(/*"@everyone"*/ "") // Want to everyone ? uncomment it
               : QByteArray("")) + "\","

                       "\"embeds\": ["
                         "{"
                           "\"title\": \"Act4 status\","
                           "\"description\": \"Server " + serveur.toUtf8() + "\\n\","
                                              //+ pseudonym.toUtf8() + " " + code.toUtf8() + "`\"," // Want to display bot pseudonym + code ? uncomment it
                           "\"color\": " + color.toUtf8() + ","
                           "\"fields\": ["
                             "{"
                               "\"name\": \"Angel :\","
                               "\"value\": \"" + valueField1.toUtf8() + "\","
                               "\"inline\": true"
                             "},"
                             "{"
                               "\"name\": \"Demon :\","
                               "\"value\": \"" + valueField2.toUtf8() + "\","
                               "\"inline\": true"
                             "}"
                           "],"
                           "\"thumbnail\": {"
                             "\"url\": \"" + thumbnail.toUtf8() + "\""
                           "},"
                           "\"footer\": {"
                             "\"text\": \"" + footerDate.toUtf8() + "\","
                             "\"icon_url\": \"" + icone.toUtf8() + "\""
                           "}"
                         "}"
                       "]"
                     "}";
    if(wHook == nullptr)
    {
        wHook = new WebHook(wHookAdd);
        wHook->sendMsg(json);
        mustBeTreated = false;
    }
    else if(mustBeTreated)
    {
        wHook->sendMsg(json);
        mustBeTreated = false;
    }
    if(currentTime > 0)
        currentTime--;
    else if(timerFcPacket->isActive())
    {
        timerFcPacket->stop();
    }
    forceRefresh = false;
}
