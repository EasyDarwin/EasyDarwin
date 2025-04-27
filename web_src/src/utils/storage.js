export const TokenInfo = 'TOKEN_INFO';

export function getTokenStorage() {
    return localStorage.getItem(TokenInfo);
}

export function removeTokenStorage() {
    localStorage.removeItem(TokenInfo);
}

export const setTokenStorage = (value) => {
    localStorage.setItem(TokenInfo,value)
};