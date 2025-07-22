FROM ubuntu:24.04

# Install only the needed runtime libraries
RUN apt-get update && apt-get install -y \
    libqt6core6 libqt6gui6 libqt6widgets6 libqt6websockets6 \
    libgomp1 libomp5 libarmadillo-dev libgsl27 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory inside the container
WORKDIR /app

# Copy the compiled binary and resource folder
COPY OHQuery ./OHQuery
COPY resources/ ./resources/
COPY config.json ./config.json

# Make binary executable
RUN chmod +x OHQuery

# Expose ports used by websockets and crow server
EXPOSE 8080
EXPOSE 12345

# Run the binary
ENTRYPOINT ["./OHQuery"]

