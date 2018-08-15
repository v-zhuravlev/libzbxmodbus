# Use an official Python runtime as a parent image
FROM python:2.7-slim

COPY requirements.txt ./requirements.txt
# Install any needed packages specified in requirements.txt
RUN pip install --trusted-host pypi.python.org -r requirements.txt

# Copy the current directory contents into the container at ./
ADD  . ./

# Make port 5020 available to the world outside this container
EXPOSE 5020

# Run app.py when the container launches
CMD ["python", "app.py"]