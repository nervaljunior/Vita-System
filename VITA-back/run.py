
import uvicorn

if __name__ == "__main__":
    uvicorn.run("app.main:app", host="0.0.0.0", port=8000, reload=True)

""" from App import app
import os 

if __name__ == 'main':
    porta =int(os.getenv('PORT'),'5000')
    app.run(host='0.0.0.0',port=porta) """