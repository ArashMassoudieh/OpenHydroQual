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

Run the container, publishing the port the configured server actually listens on
(adjust if ports are in use):

```bash
sudo docker run -d -p 12345:12345 --name ohquery-container ohquery-app
```

### Which ports to publish

The `Dockerfile` declares `EXPOSE 8080` and `EXPOSE 12345`, but `EXPOSE` only
documents a port. It publishes nothing. Only the ports named with `-p` are
reachable from outside the container.

Which port matters depends on `config.json`:

| `"Config"` value | Server started | Port to publish |
|---|---|---|
| `"WebSockets"` (current default) | Qt WebSocket server | `-p 12345:12345` |
| `"FlaskType"` | crow HTTP server | `-p 8080:8080` |

`main.cpp` starts exactly one of the two, so publishing `8080` alongside
`12345` accomplishes nothing while `config.json` says `"WebSockets"`: nothing
inside the container is listening there. The single `-p 12345:12345` above is
correct for the default config. Switch `config.json` to `"FlaskType"` and you
need `-p 8080:8080` instead.

### If you build with HTTPS enabled

`DEFINES += HTTPS` is commented out in `OHQuery.pro`, so the default build
serves plain `ws://` and needs no certificate. If you uncomment it, the binary
reads its certificate from paths on the host filesystem:

```
/etc/letsencrypt/live/greeninfraiq.com/fullchain.pem
/etc/letsencrypt/live/greeninfraiq.com/privkey.pem
```

Those paths do not exist inside the container, so mount them read-only and
restart the container after every certificate renewal (the binary reads the
files once at startup):

```bash
sudo docker run -d -p 12345:12345 \
  -v /etc/letsencrypt:/etc/letsencrypt:ro \
  --name ohquery-container ohquery-app
```

### How it is actually deployed on the server

The production instance does not use the names in this document. `systemd`
runs it as `ohquery.service`, with the container named `ohquery_container`
(underscore) from the Docker Hub image `enviroinformatics/ohquery-app:latest`,
and it mounts the temporary folder:

```bash
docker run --name ohquery_container \
  -e OHQUERY_TEMP_PATH=/home/ubuntu/OHQueryTemporaryFolder \
  -v /home/ubuntu/OHQueryTemporaryFolder:/home/ubuntu/OHQueryTemporaryFolder \
  -p 12345:12345 enviroinformatics/ohquery-app:latest
```

Use `sudo systemctl restart ohquery` there, not a bare `docker run`.

---

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

