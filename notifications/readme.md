Meteos Notifications

Service that periodically fetches environment data from Backend and notifies subscribers if readings go beyond optimal range.

Currently there's only one channel for subscribing and delivering notifications - [Telegram bot](https://core.telegram.org/bots).

Build & run in docker
---

Prerequisites: (1) [docker](https://www.docker.com/), (2) git, (3) unix shell (e.g. bash).

```
git clone https://github.com/art-in/meteos
cd meteos/notifications

./docker/scripts/build.sh        # build inside container
./docker/scripts/run.sh          # run inside container
./docker/scripts/start.sh        # build & run
```


Run prebuilt docker image
---

Prerequisites: (1) [docker](https://www.docker.com/).

```
# download image from docker hub and run it (set params first)
docker run -di \
  --name meteos-notifications \
  --restart unless-stopped \
  --mount type=bind,src=<host_dir>/config.toml,dst=/opt/project/config.toml \
  --mount type=bind,src=<host_dir>/,dst=/opt/project/db/ \
  artinphares/meteos-notifications
```