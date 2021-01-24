#include <QCoreApplication>
#include <bot.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Bot bot(argc, argv);
    return a.exec();
}
