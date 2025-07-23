#!/bin/bash

# Exit immediately if any command fails
set -e

echo "🔧 Building Qt project in release mode..."
qmake6 OHQuery.pro CONFIG+=release

echo "🔨 Compiling with $(nproc) threads..."
make -j"$(nproc)"

echo "🐳 Building Docker image 'ohquery-app'..."
sudo docker build -t ohquery-app .

echo "🏷️ Tagging image as 'enviroinformatics/ohquery-app:latest'..."
sudo docker tag ohquery-app enviroinformatics/ohquery-app:latest

echo "🔐 Logging in to Docker Hub..."
sudo docker login

echo "📤 Pushing image to Docker Hub..."
sudo docker push enviroinformatics/ohquery-app:latest

echo "✅ Done: Docker image pushed to enviroinformatics/ohquery-app:latest"
