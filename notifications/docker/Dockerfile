# make sure versions stay in sync with dev image (.devcontainer/Dockerfile)
FROM rust:1.61

# copy source files into image (filtered with .dockerignore)
COPY . /opt/project/
WORKDIR /opt/project/

# build project
RUN scripts/build.sh

# cleanup files unnecessary for runtime to reduce image size
RUN rustup self uninstall -y
RUN apt autoremove -y
RUN rm -rf src/
RUN rm -rf target/release/*/

# set entry (forwards args from cli)
ENTRYPOINT ["bash", "/opt/project/scripts/run.sh"]