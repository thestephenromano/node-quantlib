from ubuntu:19.10

# Install dependencies
RUN apt-get update
RUN apt-get upgrade -y

#
# Build Node.js
#
ARG NODEJS_VERSION=v13.10.1
RUN apt-get install -y git-core curl build-essential openssl libssl-dev python && \
    git clone https://github.com/nodejs/node.git && \
    cd node && \
    git checkout tags/${NODEJS_VERSION} -b ${NODEJS_VERSION} && \
    ./configure && \
    make && \
    make install && \
    cd .. && \
    rm -r node

# Install QuantLib
RUN apt-get install -y libquantlib0-dev

# A wildcard is used to ensure both package.json AND package-lock.json are copied
# where available (npm@5+)
COPY package*.json ./

#RUN npm install
# If you are building your code for production
RUN npm ci --only=production

# Bundle app source
COPY . .

EXPOSE 8081
CMD [ "node", "index.js" ]