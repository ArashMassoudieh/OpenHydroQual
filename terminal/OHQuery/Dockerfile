# Stage 1: Build stage
FROM ubuntu:24.04 AS builder

LABEL description="Build stage for OHQServer"

# Install required build tools and libraries
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6core6-dev \
    libblas-dev \
    liblapack-dev \
    libarmadillo-dev \
    libgsl-dev \
    cmake \
    git \
    qt6-tools-dev-tools \
    libomp-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy project files
COPY . .

# Build the project
RUN qt6-qmake OHQServer.pro && make -j$(nproc)

# Strip binary to reduce size
RUN strip OHQServer

# Stage 2: Runtime stage
FROM debian:bookworm-slim

LABEL description="Minimal runtime image for OHQServer"

# Install only required runtime libraries
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libqt6core6 \
    libgomp1 \
    libarmadillo11 \
    liblapack3 \
    libblas3 \
    libgsl27 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy stripped binary from builder
COPY --from=builder /build/OHQServer .

# Expose the Crow API port
EXPOSE 8080

# Launch the app
CMD ["./OHQServer"]

