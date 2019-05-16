Meteos Frontend.

Client-server web app that fetches environment data from Backend and shows it in browser.

---

Build & run in docker
---

Prerequisites: (1) [docker](https://www.docker.com/), (2) git, (3) unix shell (e.g. bash).

```
git clone https://github.com/artin-phares/meteos
cd meteos/frontend

./docker/scripts/build.sh        # build inside container
                                 # --clean - skip cache

./docker/scripts/run.sh          # run inside container
                                 # --backend-url (required)
                                 # --dev - rebuild on src/ changes

./docker/scripts/start.sh        # build & run
```

Notes:  
On linux, if docker requires `sudo` - setup it to run without `sudo` (see [instructions](https://askubuntu.com/a/477554/950607)).

---

Run prebuilt docker image
---

Prerequisites: (1) [docker](https://www.docker.com/).

```
# download image from docker hub and run it (set port, TODO: pass backend-url)
docker run -di -p <port>:3000 artinphares/meteos-frontend
```
