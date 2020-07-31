SOURCES = documentation.md
SOURCEFORMAT = markdown
# EXTENSIONS = fenced_code_blocks+implicit_figures+fenced_code_attributes+raw_attribute+backtick_code_blocks+multiline_tables
EXTENSIONS = implicit_figures+raw_attribute+multiline_tables
INPUT = documentation
OUTPUT=$(INPUT).tex
PDF_OUTPUT = $(INPUT).pdf
READER = evince

all:
	pandoc -s --template eisvogel.latex --biblatex --from=$(SOURCEFORMAT)+$(EXTENSIONS) --listings --template=eisvogel $(SOURCES) -o $(OUTPUT)
	pdflatex $(INPUT);biber $(INPUT);pdflatex $(INPUT)

show: all
	$(READER) $(PDF_OUTPUT) &

count_words: all
	pdftotext $(PDF_OUTPUT) - | wc -w
