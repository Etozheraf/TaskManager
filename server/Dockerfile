# Use a base image with Clang and CMake
FROM ubuntu:22.04 AS build

# Build arguments from .env
ARG BUILD_TARGET=TaskManagerServer
ARG BUILD_DIR=build

# Install dependencies
RUN apt-get update && apt-get install -y \
    clang \
    cmake \
    libboost-all-dev \
    build-essential

# Set the working directory
WORKDIR /app

# Copy .env file for build configuration
COPY .env .

# Copy the source code
COPY . .

# Create a build directory
RUN mkdir ${BUILD_DIR}
WORKDIR /app/${BUILD_DIR}

# Configure and build the project with Clang
RUN cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
RUN cmake --build .

# Final stage
FROM debian:bullseye-slim

# Build arguments from .env
ARG BUILD_TARGET=TaskManagerServer
ARG BUILD_DIR=build

# Install runtime dependencies
RUN apt-get update && apt-get install -y libboost-all-dev

# Copy the executable from the build stage
COPY --from=build /app/${BUILD_DIR}/src/${BUILD_TARGET} /usr/local/bin/${BUILD_TARGET}

# Expose the port the server listens on
EXPOSE 8080

# Run the server
CMD ["${BUILD_TARGET}"]
