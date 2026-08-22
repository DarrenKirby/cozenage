# ==========================================
# Stage 1: builder
# ==========================================
FROM alpine AS builder

# Install all the build dependencies
RUN apk add --no-cache build-base linux-headers gc-dev icu-dev cmake pkgconfig gmp-dev openssl-dev

# Set up a working directory
WORKDIR /src

# Copy source code into the container
COPY . .

# Build and install the software
RUN make
RUN make install DESTDIR=/install_root

# ==========================================
# Stage 2: final image
# ==========================================
FROM alpine

# Install ONLY the runtime libraries
RUN apk add --no-cache gc icu-libs gmp openssl

# Copy the compiled binary from the 'builder' stage into this final image
COPY --from=builder /install_root/usr/local /usr/local

# Run cozenage when the container starts
ENTRYPOINT ["cozenage"]

