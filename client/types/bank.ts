export interface Utente {
  id: number
  username: string
  nome: string
  cognome: string
  email: string
  telefono: string
  data_nascita: string
  token?: string
}

export interface Conto {
  iban: string
  saldo: number
  tipo: number
  data_apertura: string
}

export interface Transazione {
  id: number
  tipo: number
  importo: number
  descrizione: string
  data: string
  iban_controparte: string
}

export interface ApiResponse<T = unknown> {
  status: 'ok' | 'error'
  data?: T
  message?: string
}
