TEMPLATE = subdirs

SUBDIRS += \
    Bushy \
    Test

Test.depends = Bushy
