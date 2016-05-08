TEMPLATE = subdirs

SUBDIRS += \
    Bushy \
    Test \
    Benchmark

Test.depends = Bushy
