# Updating the OHQuery Docker Container

This document outlines how to rebuild and run your OHQuery Docker container after making changes to the source code.

---

## 🛠️ Step 1: Recompile the Executable

Make sure you have built the **release version** of the OHQuery executable using `qmake` and `make`:

```bash
cd ~/Projects/OpenHydroQual
qmake CONFIG+=release
make -j$(nproc)
```

This will generate the updated `OHQuery` binary in the root directory.

---

## 📦 Step 2: Rebuild the Docker Image

From the root project directory (`~/Projects/OpenHydroQual`), rebuild the Docker image:

```bash
sudo docker build -t ohquery-app .
```

---

## 🚀 Step 3: Run the Docker Container

Run the container with exposed ports (adjust if ports are in use):

```bash
sudo docker run -d -p 12345:12345 --name ohquery-container ohquery-app

```

## 📁 Notes

- Make sure `resources/` and `config.json` are present in the same directory as the `OHQuery` binary before building.
- To stop the container, press `Ctrl + C` **twice** or use `docker ps` + `docker stop <container_id>`.
- You can also add `ENTRYPOINT ["./OHQuery"]` or a custom `entrypoint.sh` if needed.

---

## 🧼 Optional: Clean Previous Containers

```bash
docker ps -a        # List containers
docker rm <id>      # Remove a container
docker rmi ohquery-app  # Remove the image if needed
```

---

## ✅ Done

You’ve now rebuilt and deployed your updated OHQuery container.

to stop the container
sudo docker stop ohquery-container

Save image to tar file: 
sudo docker save -o ohquery-app.tar ohquery-app


Pushing to DockerHub: 

docker tag ohquery-app enviroinformatics/ohquery-app:latest
docker login
docker push enviroinformatics/ohquery-app:latest

Pulling from Dockerhub: 
docker pull enviroinformatics/ohquery-app:latest

