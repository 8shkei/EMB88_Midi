#MakeFile C,C++用 SCodeにてソースファイルの拡張子選択
SCode	 := c

CC        = avr-gcc

LDFLAGS   = -lwinmm
LIBS      =
INCLUDE   = -I/mingw64/avr/include/avr -I/mingw64/include -I./include
TARGETS   = Project.elf
TARGETDIR = .
SRCROOT   = ./src
OBJROOT   = ./obj

TARGETMCU = atmega88pa

CXXFLAGS  = -O2 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -mmcu=$(TARGETMCU) -c
CXXFLAGS2 = -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=$(TARGETMCU)
RM=rm
MENU=windres

# ソースディレクトリの中を(shellの)findコマンドで走破してサブディレクトリまでリスト化する
SRCDIRS  := $(shell find $(SRCROOT) -type d)
# ソースディレクトリを元にforeach命令で全cppファイルをリスト化する
SOURCES   = $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.${SCode}))#$()だとエラーでた
# 上記のcppファイルのリストを元にオブジェクトファイル名を決定
OBJECTS   = $(subst $(SRCROOT),$(OBJROOT), $(SOURCES:.${SCode}=.o))
# ソースディレクトリと同じ構造で中間バイナリファイルの出力ディレクトリをリスト化
OBJDIRS   = $(addprefix $(OBJROOT)/, $(SRCDIRS))
# 依存ファイル.dを.oファイルから作る
DEPENDS  := $(OBJECTS:.o=.d)


# 依存ファイルを元に実行ファイルを作る
$(TARGETS): $(OBJECTS) $(LIBS)
	avr-gcc -o $(TARGETDIR)/$@ $^ $(CXXFLAGS2)
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures $@ $(@:.elf=.hex)

# 中間バイナリのディレクトリを掘りながら.cppを中間ファイル.oに
$(OBJROOT)/%.o: $(SRCROOT)/%.${SCode}
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CC) -o $@ -c $< $(CXXFLAGS) -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"  $(INCLUDE)


# 依存関係ファイル
-include $(DEPENDS)

install:$(TARGETS)
	@echo Writing now...
	@cmd /c "con2com com6 38400 $(TARGETS:.elf=.hex) /"
clean:
	$(RM) $(OBJECTS) $(TARGETS) $(TARGETS:.elf=.hex) $(DEPENDS)
write:
	cmd /c "con2com.cmd $(TARGETS:.elf=.hex)"
.PHONY: all clean