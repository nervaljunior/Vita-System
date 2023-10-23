from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.routers import apidados, apilogin  

app = FastAPI()


app.include_router(apidados.router)
app.include_router(apilogin.router)