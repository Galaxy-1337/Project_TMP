#include "headers/ui/authwindow.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDebug>

AuthWindow::AuthWindow(QWidget *parent) :
    QWidget(parent),
    loginLineEdit(new QLineEdit(this)),
    passwordLineEdit(new QLineEdit(this)),
    actionButton(new QPushButton("Войти", this)),
    toggleButton(new QPushButton("Зарегистрироваться", this)),
    titleLabel(new QLabel("Вход", this)),
    layout(new QVBoxLayout(this))
{
    qDebug() << "AuthWindow: Constructor called.";
    // Установка компоновки
    layout->addWidget(titleLabel);
    layout->addWidget(loginLineEdit);
    layout->addWidget(passwordLineEdit);
    layout->addWidget(actionButton);
    layout->addWidget(toggleButton);
    // Установка режима скрытия ввода для поля пароля
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    // Подключение сигналов к слотам для входа и регистрации
    connect(actionButton, &QPushButton::clicked, this, [this]() {
        qDebug() << "AuthWindow: Action button clicked. Title:" << titleLabel->text();
        if (titleLabel->text() == "Вход") {
            emit authRequested(loginLineEdit->text(), passwordLineEdit->text());
        } else {
            emit registerRequested(loginLineEdit->text(), passwordLineEdit->text());
        }
    });
    connect(toggleButton, &QPushButton::clicked, this, [this]() {
        qDebug() << "AuthWindow: Toggle button clicked. Current title:" << titleLabel->text();
        if (titleLabel->text() == "Вход") {
            showRegister();
        } else {
            showLogin();
        }
    });
    setLayout(layout);
}

AuthWindow::~AuthWindow() {
    qDebug() << "AuthWindow: Destructor called.";
    // No need to delete layout or widgets, as they are parented to this widget
}

void AuthWindow::showLogin() {
    qDebug() << "AuthWindow: Showing login form.";
    titleLabel->setText("Вход");
    actionButton->setText("Войти");
    toggleButton->setText("Зарегистрироваться");
    loginLineEdit->clear();
    passwordLineEdit->clear();
}

void AuthWindow::showRegister() {
    qDebug() << "AuthWindow: Showing registration form.";
    titleLabel->setText("Регистрация");
    actionButton->setText("Зарегистрироваться");
    toggleButton->setText("Войти");
    loginLineEdit->clear();
    passwordLineEdit->clear();
}
