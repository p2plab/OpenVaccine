#ifndef SENDMAIL_H
#define SENDMAIL_H

#include <QObject>

class SendMail : public QObject
{
    Q_OBJECT
public:
    explicit SendMail(QObject *parent = 0);

signals:

public slots:
};

#endif // SENDMAIL_H
