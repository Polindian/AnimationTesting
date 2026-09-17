FROM gcr.io/distroless/cc-debian10:nonroot

COPY --chown=nonroot ./server /home/nonroot/server

ENTRYPOINT ["/home/nonroot/server/AnimationTesting/Binaries/Linux/AnimationTestingServer", "AnimationTesting"]

