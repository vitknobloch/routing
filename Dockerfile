# Start with the Ubuntu Noble base image
FROM ubuntu:noble

# Set non-interactive mode to avoid prompts during installations
ENV DEBIAN_FRONTEND=noninteractive

# Update the package list and install required tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    nodejs \
    npm \
    python3 \
    python3-pip \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Verify installations
RUN node -v && npm -v && python3 --version && cmake --version && g++ --version

# Copy the typescript files from the repository intoto the container
COPY tsconfig.json /workspace/
COPY package.json /workspace/
COPY scripts /workspace/scripts/
COPY node_modules /workspace/node_modules/

# Install Node.js dependencies and compile TypeScript
RUN if [ -f package.json ]; then \
        npm install && \
        npm i --save-dev @types/node && \
        npx tsc; \
    fi

# Copy the heuristics portfolio from the repository into the container
COPY lib/ /workspace/lib/
COPY include/ /workspace/include/
COPY src/ /workspace/src/
COPY CMakeLists.txt /workspace/


# Build the heuristics portfolio
RUN rm -rf build && mkdir build && cd build && cmake .. && make && cd ..

# Set default shell to bash
SHELL ["/bin/bash", "-c"]

# Set working directory (optional)
WORKDIR /workspace

# Default entry point (optional) - update this to run your app or provide a shell
CMD ["bash"]