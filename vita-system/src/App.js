import React from "react";
import { Login } from "./pages/login/login"
import { Adm } from "./pages/adm/adm"
import { Cadastro } from "./pages/cadastro/cadastro"
import { Funcionario } from "./pages/func/func"
import { BrowserRouter as Router, Routes, Route } from "react-router-dom"
import { ProtectedRoute, ProtectedRouteFunc } from './Auth/ProtectedRoute';
import { AuthProvider } from "./Auth/context";
import { Help } from "./pages/Help/help";
import { HelpDados } from "./pages/Help/helpdados";
import { HelpRelatorio } from "./pages/Help/helprelatorio";
import { HelpSenha } from "./pages/Help/helpsenha";


function App() {
    return ( 
        <Router >
            <AuthProvider >
                <Routes >
                    <Route path = "/"
                        element = {
                        <Login />
                    }/>

                    <Route path = "*"
                        element = {
                        <Login />
                        }/>

                    <Route path = "/adm"
                        element = {
                        <ProtectedRoute>
                            <Adm />
                        </ProtectedRoute>
                    }/>

                    <Route path = "/adm/cadastro"
                        element = {
                        <ProtectedRoute>
                            <Cadastro />
                        </ProtectedRoute>
                    }/> 
                    <Route path = "/func"
                        element = {
                       <ProtectedRouteFunc>
                            <Funcionario />
                       </ProtectedRouteFunc>
                        
                    }/>
                    <Route path = "/help"
                        element = {
                       <ProtectedRouteFunc>
                            <Help />
                       </ProtectedRouteFunc>
                        
                    }/>
                    <Route path = "/helpdados"
                        element = {
                        <ProtectedRouteFunc>
                            <HelpDados />
                        </ProtectedRouteFunc>   
                        
                    }/>
                    <Route path = "/helprelatorio"
                        element = {
                        <ProtectedRouteFunc>
                            <HelpRelatorio />
                       </ProtectedRouteFunc>   
                        
                    }/>
                    <Route path = "/helpsenha"
                        element = {
                        <ProtectedRouteFunc>
                            <HelpSenha />
                        </ProtectedRouteFunc>   
                        
                    }/>

                </Routes> 
            </AuthProvider > 
        </Router>
    );
}

export default App;