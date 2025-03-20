# Устанавливаем базовый образ
FROM ubuntu:22.04

# Устанавливаем временную зону
ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Обновление и установка зависимостей
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    build-essential \
    qtchooser \
    qt6-base-dev-tools \
    qmake6

# Копируем файлы проекта в контейнер
WORKDIR /app
COPY . /app

# Проверяем, что файлы скопированы (опционально, для отладки)
RUN ls -la /app

# Сборка проекта
RUN qmake6 /app/echoServer.pro && make

# Запускаем сервер
WORKDIR /app
ENTRYPOINT ["./echoServer"]



/// docker build -t my-qt-server .
///docker run -it --rm my-qt-server