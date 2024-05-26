# JKLMNOP! :)
FROM alpine

COPY . /mp2
COPY . /mp7
WORKDIR /mp7
RUN apk update
RUN apk add py-pip

RUN pip install -r requirements.txt
