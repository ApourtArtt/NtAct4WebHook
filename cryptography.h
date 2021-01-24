#ifndef Cryptography_H
#define Cryptography_H
#include <QObject>

class Cryptography : public QObject
{
    Q_OBJECT
public:
    explicit Cryptography(QObject *parent = nullptr);
    QByteArray encryptGamePacket(const QString& buf, int session, bool is_session_packet = 0) const;
    std::vector<unsigned char> encryptLoginPacket(const QString& buf) const;
    QString decryptLoginPacket(const QByteArray& ba, std::size_t len) const;
    QStringList decryptGamePacket(const QByteArray& inp, std::size_t len);
    QString createLoginHash(const QString& user) const;
    QString createLoginVersion(void) const;
    int randomNumber(int min, int max) const;
    QString Sha512(const QString plainText) const;
    QByteArray createLoginPacket(const QString& username, const QString& password, const QString& langCode) const;
    void randomize() const;
    static void setGFuid(const QString &value);
    static QString getLoginPacket();
    void setClientXhash(QString hash);
    void setClienthash(QString hash);
    QString fileChecksum(const QString &fileName);
    void setVersion(QString vers);
private:

    static QString clientXhash;
    static QString clienthash;
    static QString version;
    static QString gfuid;
    static QString loginPacket;
    void completeGamePacketEncrypt(std::vector<unsigned char>& buf, int session, bool is_session_packet = 0) const;
    QByteArray bigPacket;
    std::string last_current_packet;
    std::vector<unsigned char> last_buf;
    QString pwEncrypt(QString pw) const;
};
#endif // Cryptography_H
