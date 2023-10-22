import './adm.css'
import React, { useEffect, useState } from "react";
import axios from "axios";
import { Navigate, useNavigate } from 'react-router-dom';
import { useAuth } from '../../Auth/context';


export const Adm = () => {

    const logout = useAuth();  
    const navigate = useNavigate();
    const handleCadastro = () => {
    navigate('/adm/cadastro')
    }

    const [users, setUsers] = useState([]);

  useEffect(() => {
    fetchUsers();
  }, []);

  const fetchUsers = async () => {
    try {
      const response = await axios.get("http://localhost:5000/users");
      const usersData = response.data;
      setUsers(usersData);
    } catch (error) {
      console.error("Erro ao buscar usuários:", error);
    }
  };

  const getRoleLabel = (role) => {
    return role === 1 ? "Administrador" : "Operador";
  };

  

  const handleDeleteUser = async (userId) => {
    try {
      await axios.delete(`http://localhost:5000/users/${userId}`);
      fetchUsers(); // Atualiza a lista de usuários após a exclusão
    } catch (error) {
      console.error("Erro ao excluir usuário:", error);
    }
  };

  const handleLogout = () => logout.logout();

  return (
    <div id="admbox" className="box">
      <table>
        <thead>
          <tr>
            <th>Usuário</th>
            <th>Matrícula</th>
            <th>Função</th>
          </tr>
        </thead>
        <hr></hr>
        <tbody>
            {users.map((user) => (
              <tr key={user.id}>
                <td>{user.username}</td>
                <td>{user.id}</td>
                <td>{getRoleLabel(user.role)}</td>
                <td>
                  <button
                    type="button"
                    className="delete-button"
                    onClick={() => handleDeleteUser(user.id)}>
                    Excluir
                  </button>
                </td>
              </tr>
            ))}
        </tbody>
      </table>
      <button 
        type='button' 
        id='buttonadm'
        onClick={handleCadastro}>
        NOVO USUÁRIO</button>
        <button type="submit" id="buttonlogout" onClick={handleLogout}>
          SAIR
        </button>
    </div>
    )
}

