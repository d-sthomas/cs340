import re
from flask import Flask, render_template, request, jsonify
import json
import os
import requests

from datetime import datetime, timedelta, time

app = Flask(__name__)


cache = dict()
def next_weekday(d, weekday, hour, min):
    days_ahead = weekday - d.weekday()
    if days_ahead <= 0: # Target day already happened this week
      if d.time() > time(hour, min):
        days_ahead += 7
    return d + timedelta(days_ahead)

# Route for "/" (frontend):
@app.route('/')
def index():
  return render_template("index.html")


# Route for "/weather" (middleware):
@app.route('/weather', methods=["POST"])
def POST_weather():
  course = request.form["course"]
  course.rstrip().lstrip()
  length = len(course)
  number = course[length - 3: length] # number is always last 3 digits
  subject = course[0: length-3].rstrip() # gets subject up to space or number
  
  server_url = os.getenv('COURSES_MICROSERVICE_URL')
  r = requests.get(f'{server_url}/{subject.upper()}/{number}/')
  if r.status_code == 404: return {}, 400 # invalid course or no data
  course_json = r.json()
  course_days = course_json['Days of Week']
  course_time = course_json['Start Time']
  if len(course_days) == 0: return (), 400
  # splits meeting time into hour and mins
  hour = int(course_time.partition(":")[0])
  min_ = course_time.partition(":")[2]
  min = int(min_[0:2])
  half = min_[3:5]
  if (half == "PM"):
    if (hour != 12): hour += 12 # converts to military time
  
  today_date = datetime.now()

  valid_days = [] # adds potential next meeting days
  if "M" in course_days: valid_days.append(next_weekday(today_date, 0, hour, min))
  if "T" in course_days: valid_days.append(next_weekday(today_date, 1, hour, min))
  if "W" in course_days: valid_days.append(next_weekday(today_date, 2, hour, min))
  if "R" in course_days: valid_days.append(next_weekday(today_date, 3, hour, min))
  if "F" in course_days: valid_days.append(next_weekday(today_date, 4, hour, min))

  next_meeting = valid_days[0]
  for day in valid_days:
    if day < next_meeting:
      next_meeting = day
  
  next_meeting_time = next_meeting.replace(hour=hour, minute=min, second=0, microsecond=0)
  if subject.upper() + number in cache:
    return cache[subject.upper() + number]
  
  if (min > 30): 
    converted_time = next_meeting_time.replace(hour=hour+1, minute=0, second=0, microsecond=0)
  else:
    converted_time = next_meeting_time.replace(hour=hour, minute=0, second=0, microsecond=0)
  weather_url = "https://api.weather.gov/gridpoints/ILX/95,71/forecast/hourly"
  temp = ""
  forecast = ""
  diff = next_meeting - today_date
  days, seconds = diff.days, diff.seconds
  hours = days * 24 + seconds // 3600
  if (hours > 144):
    temp = "forecast unavailable"
    forecast = "forecast unavailable"
  else :
    w = requests.get(weather_url)
    course_weather = w.json()
    properties = course_weather.get('properties', {}).get('periods')

    tmp = converted_time.strftime("%Y-%m-%d") + 'T' + converted_time.strftime("%H:%M:%S") + "-05:00"
    for p in properties:
      if (p.get('startTime') == tmp):
        temp = p.get('temperature')
        forecast = p.get('shortForecast')
  print("meeting: " + str(next_meeting_time))
  value = {
    "course": course_json['course'],
    "nextCourseMeeting": str(next_meeting_time),
    "forecastTime": str(converted_time),
    "temperature": temp,
    "shortForecast": forecast
  }
  cache[subject.upper() + number] = value
  return value, 200


# Route for "/weatherCache" (middleware/backend):
@app.route('/weatherCache')
def get_cached_weather():
  
  return cache, 200
