FROM ubuntu:22.04

# Установим базовые пакеты
RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev \
    wget unzip \
    libxext6 libxrender1 libxtst6 libxi6 libgtk-3-0 \
    mc \
    && rm -rf /var/lib/apt/lists/*

# for VSCode
RUN apt-get update && apt-get install -y \
    curl \
    gpg \
    git \
    gnupg2 \
    software-properties-common \
    ca-certificates \
    apt-transport-https \
    libx11-xcb1 \
    libxkbfile1 \
    libsecret-1-0 \
    libnss3 \
    libgconf-2-4 \
    libasound2 \
    libatk-bridge2.0-0 \
    libgtk-3-0 \
    gdb \
    libssl-dev \
    && apt-get clean

# For system project
RUN apt-get update && apt-get install -y \
    ninja-build \
    libglfw3 libglfw3-dev \
    libfreetype6 libfreetype6-dev \
    libglew-dev

#################################################################

# Создаём пользователя 'dev' с домашним каталогом
RUN useradd -ms /bin/bash dev \
    && echo "dev:edgo45@C*" | chpasswd \
    && usermod -aG sudo dev \
    && chown -R dev:dev /home/dev

# Устанавливаем рабочую директорию
WORKDIR /home/dev

#################################################################

# создаем директорию /project/system
RUN mkdir -p \
    /home/dev/project/system \
    && chown -R dev:dev /home/dev

#################################################################

# Устанавливаем пользователя по умолчанию
USER dev

# Запуск bash по умолчанию
CMD [ "bash" ]