FROM meteos-backend-tools

ARG BUILD_TEST=OFF

# set entry (forwards --test arg from cli)
ENTRYPOINT ["bash", "/opt/project/build/run-bin.sh"]

COPY . /opt/project/

# generate build system and build the project
WORKDIR /opt/project/build
RUN cmake -S . -B ./build-system -DBUILD_TEST=$BUILD_TEST
RUN cmake --build ./build-system --config Release
RUN mkdir -p /opt/project/build/bin/data/
