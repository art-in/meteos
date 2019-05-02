Meteos Backend.

Provides rest api for storing environment data from Sensor and serving it to Frontend.

---

Build & run in docker
---

Prerequisites: (1) [docker](https://www.docker.com/), (2) git, (3) unix shell (e.g. bash).

```
git clone https://github.com/artin-phares/meteos
cd ./meteos/backend

./scripts/build.sh    # build inside container
                      # --test  - build tests
                      # --tools - rebuild tools

./scripts/run.sh      # run inside container
                      # --test  - run tests

./scripts/start.sh    # build & run
./scripts/test.sh     # build & run tests
```

Notes:  
On linux, if docker requires `sudo` - setup it to run without `sudo` ([instructions](https://askubuntu.com/a/477554/950607)).

---

Run prebuild docker
---

Prerequisites: (1) [docker](https://www.docker.com/).

```
# download image from docker hub and run it (set port and folder path to save database in)
docker run -di \
    -p <port>:8080 \
    --mount type=bind,src=<database folder path>,dst=/opt/project/build/bin/data/ \
    artinphares/meteos-backend
```

Notes:  
Database folder path should be (1) absolute, (2) exist, (3) windows format: "/d/my/folder/".
