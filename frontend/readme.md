Meteos Frontend.

Client-server web app that fetches environment data from Backend and shows it in browser.

---

Build & run in docker
---

Prerequisites: (1) [docker](https://www.docker.com/), (2) git, (3) unix shell (e.g. bash).

```
git clone https://github.com/art-in/meteos
cd meteos/frontend

./docker/scripts/build.sh        # build inside container
                                 # --clean - skip cache

./docker/scripts/run.sh          # run inside container
                                 # --backend-url (required)
                                 # --dev - rebuild on src/ changes
                                 # --log-folder - folder to drop log file to
                                 # --tls-folder - folder to read tls certificate from
                                 # --tls-key - file name of certificate private key
                                 # --tls-cert - file name of certificate

./docker/scripts/start.sh        # build & run
```

Notes:  
On linux, if docker requires `sudo` - setup it to run without `sudo` (see [instructions](https://askubuntu.com/a/477554/950607)).

---

Run prebuilt docker image
---

Prerequisites: (1) [docker](https://www.docker.com/).

```
# download image from docker hub and run it (set params first)
docker run -di \
  --restart unless-stopped \
  -p <port>:3001 \
  --mount type=bind,src=<log folder>,dst=/opt/log \
  --mount type=bind,src=<cert folder>,dst=/opt/cert \
  artinphares/meteos-frontend \
  --backend-url=<host/port> \
  --tls-key=<cert key filename> \
  --tls-cert=<cert filename>
```
