# Set OS
FROM node:lts-alpine
WORKDIR /app

# Setup gcc, libraries, git and ncurses
RUN apk add build-base git make

# Get source code
RUN git clone https://github.com/sonofoven/Multichat.git Multichat
WORKDIR /app/Multichat

# Make executable
RUN make server
WORKDIR /app/Multichat/server

EXPOSE 8080

# Start server with optional server name
CMD ["sh", "-c", "./server \"$MULTICHAT_NAME\""]
