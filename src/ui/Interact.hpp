
#pragma once

#include <QMainWindow>
#include <QObject>
#include <QQuickItem>

class Interact : public QObject
{
    Q_OBJECT
public:
    explicit Interact(QObject *parent = 0);

signals:

public slots:
    QString connect(QString username, QString password, QString host, QString port)
    {
        return username + ":" + password + "@" + host + ":" + port;
    }
};
