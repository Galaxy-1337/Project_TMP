#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class AuthWindow : public QWidget {
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

    void showLogin();
    void showRegister();

signals:
    void authRequested(const QString &login, const QString &passHash);
    void registerRequested(const QString &login, const QString &passHash);

private:
    QLineEdit *loginLineEdit;
    QLineEdit *passwordLineEdit;
    QPushButton *actionButton;
    QPushButton *toggleButton;
    QLabel *titleLabel;
    QVBoxLayout *layout;
};

#endif // AUTHWINDOW_H
