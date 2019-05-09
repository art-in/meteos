FROM ubuntu:18.04

# set entry (forwards --dev arg from cli)
ENTRYPOINT ["bash", "/opt/project/scripts/run.sh"]

# install tools
RUN apt-get update
RUN apt-get install -y curl xz-utils

# install nodejs (through direct link since apt-get one is old)
RUN mkdir -p /opt/node
WORKDIR /opt/node
RUN curl https://nodejs.org/dist/v10.15.3/node-v10.15.3-linux-x64.tar.xz -o node-v10.15.3-linux-x64.tar.xz
RUN mkdir -p /usr/local/lib/nodejs
RUN tar -xJvf node-v10.15.3-linux-x64.tar.xz -C /usr/local/lib/nodejs
ENV PATH=/usr/local/lib/nodejs/node-v10.15.3-linux-x64/bin:$PATH

# copy project sources
COPY . /opt/project/
WORKDIR /opt/project/

# install project dependencies
RUN npm install

# build project
RUN /opt/project/scripts/build.sh