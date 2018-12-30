def application_format(name):
    application = """
+---%s---+
|   %s   |
+---%s---+
    """

    print application %('-' * len(name), name, '-' * len(name))

application_format("EchoServer")

