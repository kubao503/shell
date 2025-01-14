FROM ubuntu:24.04 AS build

RUN apt-get update -y
RUN apt-get install -y build-essential bison flex cmake

WORKDIR /app
RUN mkdir -p build

COPY . .

WORKDIR /app/build

RUN cmake ..

RUN cmake --build .

FROM ubuntu:24.04 AS final

COPY --from=build /app/build/uxsh /usr/local/bin/uxsh

CMD ["uxsh"]