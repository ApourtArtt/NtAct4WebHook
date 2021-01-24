#ifndef BOT_H
#define BOT_H

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include "cryptography.h"
#include "webhook.h"

class Bot : public QObject
{
public:
    Bot(int argc, char *argv[]);

private slots:
    void connected();
    void disconnected();
    void recievedPacket();
    void sendPulsePacket();
    void handleFcPacket();

private:
    void Initialise();
    void send(QString packet, bool loginServer = false);
    int getPid();
    void manageServChann(QString packetList);
    void manageCharacter(QString packetList);
    QStringList decrypt(QByteArray msg);
    QByteArray encrypt(QString packet,bool loginserver = false);
    QString GetRandomString(int length) const;

    QTcpSocket *socket;
    Cryptography *crypt;
    QTimer *keepAlive, *timerFcPacket;
    WebHook *wHook;

    QString wHookAdd;

    bool loginServer, loggedin, entry, passed;
    QString gameip;
    QString gfuid;
    unsigned short gameport;
    int sid, pid, runtime;
    QString userid, pseudonym;

    bool idOk, OKSent;
    QString date;
    int characterSlot;

    int mapID, posX, posY;

    QString ipLogin, username, password;
    unsigned short portLogin;
    QString clientxhash, clienthash, version;
    QString serveur, canal;

    QString packetToHandle;
    bool mustBeTreated;
    QString lastAction;

    int currentTime;
    bool forceRefresh;
    QString code;
    QString camp;
    int timeMinute;
    QString langcode;

    const QString AVANTAGEDEMON = "3355964";
    const QString AVANTAGEANGE = "3355964";
    const QString EXAEQUO = "3355964";
    const QString MUKRADEMON = "12124160";
    const QString MUKRAANGE = "12124160";
    const QString RAIDANGE = "16777113";
    const QString RAIDDEMON = "11045346";

    const QString ICONE_RA = "https://cdn.discordapp.com/attachments/605360783093661696/607377615652913187/ANGEGIF.gif";
    const QString ICONE_RD = "https://cdn.discordapp.com/attachments/605360783093661696/607377618836127791/DEMONGIF.gif";
    const QString ICONE_MUKRA = "https://cdn.discordapp.com/attachments/607377140387807332/607383552186777622/MUKRAGIF.gif";
    const QString ICONE_NO = "https://cdn.discordapp.com/attachments/607377140387807332/607379361540341805/BASEGIF.gif";

    const QString THUMBNAIL_RA = "https://cdn.discordapp.com/attachments/607377140387807332/607672046796668928/RAIDANGE.gif";
    const QString THUMBNAIL_RD = "https://cdn.discordapp.com/attachments/607377140387807332/607672054912909313/RAIDDEMON.gif";
    const QString THUMBNAIL_MUKRAANGE = "https://cdn.discordapp.com/attachments/747248122148683819/799965621105590292/MUKRAANGEg.gif";
    const QString THUMBNAIL_MUKRADEMON = "https://cdn.discordapp.com/attachments/747248122148683819/799965617641095199/MUKRADEMONg.gif";
    const QString THUMBNAIL_AVANGE = "https://cdn.discordapp.com/attachments/605360783093661696/607586715997306919/AVANTANGE.gif";
    const QString THUMBNAIL_AVDEMON = "https://cdn.discordapp.com/attachments/605360783093661696/607586755750920202/AVANTAGEDEMON.gif";
    const QString THUMBNAIL_EXAEQUO = "https://cdn.discordapp.com/attachments/605360783093661696/607595091649101834/EGAL.gif";

};

#endif // BOT_H
