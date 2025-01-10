FROM ubuntu:24.04 as build

RUN apt-get update -y
RUN apt-get install -y build-essential bison flex cmake

WORKDIR /app
RUN mkdir -p build

COPY . .

WORKDIR /app/build

RUN cmake ..

RUN cmake --build .

FROM ubuntu:24.04 as final

COPY --from=build /app/build/shell /usr/local/bin/shell

CMD shell