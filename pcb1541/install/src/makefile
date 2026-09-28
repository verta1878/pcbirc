# Makefile — PCBoard installer C reimplementation
#
# Builds install(.exe) from install.c. Cross-platform:
#   - Linux / macOS: uses gcc
#   - Windows: use `nmake -f Makefile.msc` (not yet written) or MinGW
#   - DOS (OpenWatcom): use `wmake -f Makefile.wcc` (not yet written)
#
# Runtime dep: redx binary (pcb1541/install/archivers/redx). Build it with:
#   cd ../archivers/redx && make
# Then either put it in PATH or pass its path via install --redx <path>.
#
# install v1.10.1

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wno-format-truncation -Wno-stringop-truncation \
           -Wno-unused-result
LDFLAGS ?=

TARGET   = install
SRC      = install.c

.PHONY: all clean test test-minimal

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

# Quick smoke test — needs redx built and INSTALL.zip extracted somewhere.
# Set INST_ARCHIVES and INST_REDX from the environment.
#
#   make test INST_ARCHIVES=/tmp/inst_test/archives INST_REDX=/tmp/redx
#
test: $(TARGET)
	@if [ -z "$$INST_ARCHIVES" ] || [ -z "$$INST_REDX" ]; then \
	  echo "Set INST_ARCHIVES=<dir with .RED files> and INST_REDX=<redx binary>"; \
	  echo "Example: make test INST_ARCHIVES=/tmp/archives INST_REDX=/usr/local/bin/redx"; \
	  exit 1; \
	fi
	@rm -rf /tmp/install-v1101-test && mkdir -p /tmp/install-v1101-test/target
	@echo '// v1.10.1 smoke test — minimal file ops only'                > /tmp/install-v1101-test/test.dat
	@echo '@DefineProject'                                              >> /tmp/install-v1101-test/test.dat
	@echo '  @Name = "PCBoard"'                                         >> /tmp/install-v1101-test/test.dat
	@echo '  @Version = "15.3"'                                         >> /tmp/install-v1101-test/test.dat
	@echo '  @Subdir = "\\\\PCB\\\\"'                                   >> /tmp/install-v1101-test/test.dat
	@echo '  @OutDrive = C'                                             >> /tmp/install-v1101-test/test.dat
	@echo '@EndProject'                                                 >> /tmp/install-v1101-test/test.dat
	@echo ''                                                            >> /tmp/install-v1101-test/test.dat
	@echo '@BeginLib PCBOARD.RED'                                       >> /tmp/install-v1101-test/test.dat
	@echo '  @File PCBOARD.EXE  @Out PCBOARD.EXE'                       >> /tmp/install-v1101-test/test.dat
	@echo '  @File PCBOARDM.EXE @Out PCBOARDM.EXE'                      >> /tmp/install-v1101-test/test.dat
	@echo '@EndLib'                                                     >> /tmp/install-v1101-test/test.dat
	@echo '@Exit'                                                       >> /tmp/install-v1101-test/test.dat
	./$(TARGET) -a $$INST_ARCHIVES -t /tmp/install-v1101-test/target \
	            -r $$INST_REDX /tmp/install-v1101-test/test.dat
	@echo ""
	@echo "Test tree:"
	@find /tmp/install-v1101-test/target -type f

# Minimal one-liner test that assumes the standard sandbox layout.
# For interactive use — nothing else in the repo depends on this target.
test-minimal:
	@echo "Not implemented — depends on external test env. See 'test' target."
