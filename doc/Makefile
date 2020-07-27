SOURCES = documentation.md
SOURCEFORMAT = markdown
EXTENSIONS = fenced_code_blocks+implicit_figures+fenced_code_attributes+raw_attribute+backtick_code_blocks+multiline_tables
INPUT = documentation
PDF_OUTPUT = $(INPUT).pdf
READER = evince

all:
	pandoc -s --template default.latex --biblatex --from=$(SOURCEFORMAT)+$(EXTENSIONS) --template=custom.latex $(SOURCES) -o $(PDF_OUTPUT)

show: all
	$(READER) $(PDF_OUTPUT) &

count_words: all
	pdftotext $(PDF_OUTPUT) - | wc -w
