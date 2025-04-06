FROM ubuntu:22.04

# Установим базовые пакеты
RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev \
    wget unzip \
#    openjdk-17-jdk \
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

# Установим Eclipse
RUN wget https://ftp.fau.de/eclipse/technology/epp/downloads/release/2025-03/R/eclipse-cpp-2025-03-R-linux-gtk-x86_64.tar.gz && \
    tar -xzf eclipse-cpp-*.tar.gz -C /opt && \
    ln -s /opt/eclipse/eclipse /usr/local/bin/eclipse && \
    rm eclipse-cpp-*.tar.gz

####################################################################

# Добавим репозиторий Microsoft для установки VSCode
RUN curl https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > /usr/share/keyrings/microsoft-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/microsoft-archive-keyring.gpg] https://packages.microsoft.com/repos/vscode stable main" | tee /etc/apt/sources.list.d/vscode.list \
    && apt-get update

####################################################################

# Установим Visual Studio Code
RUN apt-get install -y code

#################################################################

# директорию my
RUN mkdir -p \
    /home/dev/my/projects/system \
    /home/dev/my/projects/eclipse-workspace \
    && chown -R dev:dev /home/dev

#################################################################

# Устанавливаем пользователя по умолчанию
USER dev

#################################################################

# Склонируем репозиторий
RUN git clone https://github.com/AnatoliySolomichev/system.git /home/dev/my/projects/system

#################################################################

# Запуск bash по умолчанию
CMD [ "bash" ]

# Запустим VSCode (чтобы контейнер мог запускать графический интерфейс, нужно будет использовать X11)
#CMD ["code", "."]