#Docker

#get file:
https://github.com/AnatoliySolomichev/system/blob/main/Dockerfile

#save Dockerfile in 
~/my/projects/dockers/opengl-dev-env_1/Dockerfile

#install Docker:
sudo apt install docker.io
sudo systemctl enable --now docker
sudo usermod -aG docker $USER

cd ~/my/projects/dockers/opengl-dev-env_1/Dockerfile

#allow local docker containers to access the X server
xhost +local:docker

# build a Docker image named 'opengl-dev-env' from the current directory
docker build -t opengl-dev-env .

# run a container with GUI and OpenGL support using the 'opengl-dev-env' image
docker run -it \
  --env DISPLAY=$DISPLAY \
  --env XAUTHORITY=$XAUTHORITY \
  --volume="$HOME/.Xauthority:$XAUTHORITY" \
  --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
  --device=/dev/dri \
  --name opengl-dev \
  opengl-dev-env

#Run Eclipse
docker exec -it --user dev opengl-dev eclipse

#Run MC
docker exec -it --user dev opengl-dev mc

#start VM after "run"
docker start -ai opengl-dev

Introduction video:
https://www.youtube.com/watch?v=LORvI01v5is

Email:
system.unite.v1@gmail.com

A telegram channel for uniting people on the blockchain:
https://t.me/unite_on_blockchain

Virtual PC link:
https://drive.google.com/drive/folders/1IOUuN7PVdYZrL5Sw9vmiDhaDOA9cqbpc?usp=sharing
user: user_1
password: eic!$ufop3e5
