#===============================================================================
# Plugin Makefile for WUPS-based Wii U development
#===============================================================================

# 引用するルールの定義（WUPS用）
include $(DEVKITPRO)/wups/share/wups_rules

#===============================================================================
# ディレクトリ設定
#===============================================================================
TOPDIR ?= $(CURDIR)

WUT_ROOT   := $(DEVKITPRO)/wut
WUPS_ROOT  := $(DEVKITPRO)/wups
WUMS_ROOT  := $(DEVKITPRO)/wums

TARGET      := Stardust
BUILD       := build

ROOT_SOURCE := $(TOPDIR)/source
SOURCES := $(shell find $(ROOT_SOURCE) -type d)
SOURCES := $(foreach source,$(SOURCES),$(source:$(TOPDIR)/%=%)/)

DATA     := data
INCLUDES := include

#===============================================================================
# コンパイルオプション
#===============================================================================
CFLAGS     := -g -Wall -O2 -ffunction-sections $(MACHDEP)
CFLAGS     += $(INCLUDE) -D__WIIU__ -D__WUT__ -D__WUPS__
CXXFLAGS   := $(CFLAGS) -std=c++23
ASFLAGS    := -g $(ARCH) -mregnames
LDFLAGS    := -g $(ARCH) $(RPXSPECS) -Wl,-Map,$(notdir $*.map) $(WUPSSPECS)

LIBS       := -lwups -lwut -lnotifications

# ライブラリのルートディレクトリ
LIBDIRS    := $(PORTLIBS) $(WUPS_ROOT) $(WUT_ROOT) $(WUMS_ROOT)

#===============================================================================
# ビルドルール（BUILDディレクトリでビルド実行）
#===============================================================================
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

# パス設定
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

# ファイル収集
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

# リンカ選択（C++ファイルが存在すればCXXでリンク）
ifeq ($(strip $(CPPFILES)),)
	export LD := $(CC)
else
	export LD := $(CXX)
endif

# オブジェクトファイルとヘッダー生成対象
export OFILES_BIN  := $(addsuffix .o,$(BINFILES))
export OFILES_SRC  := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES      := $(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN  := $(addsuffix .h,$(subst .,_,$(BINFILES)))

# インクルードパス・ライブラリパス
export INCLUDE     := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                      $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                      -I$(CURDIR)/$(BUILD)
export LIBPATHS    := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

#===============================================================================
# エントリーポイント
#===============================================================================
.PHONY: all clean $(BUILD)

all: $(BUILD)

$(BUILD):
	@$(shell [ ! -d $(BUILD) ] && mkdir -p $(BUILD))
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo cleaning ...
	@rm -fr $(BUILD) $(TARGET).wps $(TARGET).elf

#===============================================================================
else  # $(BUILD) ディレクトリでの処理
#===============================================================================
.PHONY: all

DEPENDS := $(OFILES:.o=.d)

#-------------------------------------------------------------------------------
# メインターゲット
#-------------------------------------------------------------------------------
all: $(OUTPUT).wps

# 依存関係
$(OUTPUT).wps: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

$(OFILES_SRC): $(HFILES_BIN)

#-------------------------------------------------------------------------------
# バイナリファイルの変換（*.bin → .o/.h）
#-------------------------------------------------------------------------------
%.bin.o %_bin.h: %.bin
	@echo $(notdir $<)
	@$(bin2o)

# 依存関係の自動読み込み
-include $(DEPENDS)

endif
#===============================================================================
