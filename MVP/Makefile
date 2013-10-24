JFLAGS = -g -Xlint
JC = javac

.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS)  $*.java

CLASSES = \
	DLDataPointComparator_asc.java \
	DLDataPointComparator_desc.java \
	DLDataPoint.java \
	MVPComputation.java \
	MVPNode.java \

default: classes

classes: $(CLASSES:.java=.class)

clean:
	$(RM) *.class
