# Use an official Python runtime as a parent image
FROM python:2.7-slim

# Set the working directory to /modbus-server
#WORKDIR /modbus-server

COPY requirements.txt ./requirements.txt
# Install any needed packages specified in requirements.txt
RUN pip install --trusted-host pypi.python.org -r requirements.txt

# Copy the current directory contents into the container at ./
ADD  . ./

# Make port 5000 available to the world outside this container
EXPOSE 5020

# Define environment variable
#ENV JIRA_USER your_login
#ENV JIRA_PASS your_pass

# Run app.py when the container launches
CMD ["python", "app.py"]