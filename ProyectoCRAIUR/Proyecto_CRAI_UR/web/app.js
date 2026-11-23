/**
 * @file app.js
 * @brief Cliente REST offline para el panel administrativo CRAI (cero JSON crudo, auto-sesión F5, RQF 021).
 * @author Oscar David Alvarado Navarrete (DA)
 */

let sessionToken = localStorage.getItem("crai_token") || "";
const $ = (id) => document.getElementById(id);

const NOMBRES_SEDES = [
  "Claustro",
  "Quinta de Mutis",
  "Emprendimiento y GSB",
  "Sede Norte",
  "Hospital Mayor Mederi"
];

let cacheRecursos = [];
let paginaRec = 1;
const POR_PAGINA = 25;
let cooldownTimer = null;

function nombreSede(id) {
  return NOMBRES_SEDES[id] || ("Sede " + id);
}

function escapeHtml(str) {
  return String(str ?? "")
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}

function badgeEstado(estado) {
  const e = String(estado || "").toLowerCase();
  if (e === "disponible") return '<span class="badge badge-ok">Disponible</span>';
  if (e === "prestado") return '<span class="badge badge-warn">Prestado</span>';
  return '<span class="badge badge-err">Mantenimiento</span>';
}

function mostrarAlerta(mensaje, esOk = true) {
  const el = $("notificacion");
  el.textContent = mensaje;
  el.className = "toast " + (esOk ? "toast-ok" : "toast-err");
  clearTimeout(el._t);
  el._t = setTimeout(() => el.classList.add("oculto"), 4800);
}

function iniciarCuentaRegresivaCooldown(segundos) {
  clearInterval(cooldownTimer);
  let rest = parseInt(segundos, 10) || 0;
  if (rest <= 0) { $("box-cooldown").classList.add("oculto"); return; }
  $("box-cooldown").classList.remove("oculto");
  $("lbl-cooldown-seg").textContent = rest;
  cooldownTimer = setInterval(() => {
    rest--;
    if (rest <= 0) {
      clearInterval(cooldownTimer);
      $("box-cooldown").classList.add("oculto");
    } else {
      $("lbl-cooldown-seg").textContent = rest;
    }
  }, 1000);
}

async function apiFetch(ruta, metodo = "GET", cuerpo = null) {
  const opciones = { method: metodo, headers: { "Content-Type": "application/json" } };
  if (sessionToken) opciones.headers["X-Session-Token"] = sessionToken;
  if (cuerpo !== null) opciones.body = JSON.stringify(cuerpo);
  const resp = await fetch(ruta, opciones);
  const json = await resp.json();
  if (resp.status === 401 && ruta !== "/api/login" && ruta !== "/api/sesion") {
    cerrarSesionLocal();
  }
  return { status: resp.status, ...json };
}

function cerrarSesionLocal() {
  sessionToken = "";
  localStorage.removeItem("crai_token");
  $("vista-app").classList.add("oculto");
  $("barra-sesion").classList.add("oculto");
  $("vista-login").classList.remove("oculto");
}

function entrarApp(nombreAdmin) {
  $("lbl-admin").textContent = "Recepcionista: " + nombreAdmin;
  $("vista-login").classList.add("oculto");
  $("vista-app").classList.remove("oculto");
  $("barra-sesion").classList.remove("oculto");
  cargarDashboard();
}

// Auto-restauración de sesión al recargar (F5)
(async function restaurarSesionAlCargar() {
  if (!sessionToken) return;
  const r = await apiFetch("/api/sesion");
  if (r.exito && r.data && r.data.nombreCompleto) {
    entrarApp(r.data.nombreCompleto);
  } else {
    cerrarSesionLocal();
  }
})();

// --- LOGIN / LOGOUT / UNDO ---
$("form-login").addEventListener("submit", async (e) => {
  e.preventDefault();
  const r = await apiFetch("/api/login", "POST", {
    documento: $("login-doc").value,
    clave: $("login-clave").value
  });
  if (r.exito) {
    sessionToken = r.data.token;
    localStorage.setItem("crai_token", sessionToken);
    $("box-cooldown").classList.add("oculto");
    mostrarAlerta(r.mensaje, true);
    entrarApp(r.data.admin.nombre + " " + r.data.admin.apellido);
  } else {
    mostrarAlerta(r.mensaje, false);
    if (r.status === 423 && r.data && r.data.segundosRestantes) {
      iniciarCuentaRegresivaCooldown(r.data.segundosRestantes);
    }
  }
});

$("btn-logout").addEventListener("click", async () => {
  await apiFetch("/api/logout", "POST");
  cerrarSesionLocal();
  mostrarAlerta("Sesión cerrada.", true);
});

$("btn-undo").addEventListener("click", async () => {
  const r = await apiFetch("/api/undo", "POST");
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) {
    await cargarDashboard();
    await cargarRecursos();
    await cargarPrestamos();
    await cargarUsuarios();
  }
});

// --- NAVEGACIÓN LATERAL ---
document.querySelectorAll("#nav-sidebar button").forEach((btn) => {
  btn.addEventListener("click", () => {
    document.querySelectorAll("#nav-sidebar button").forEach((b) => b.classList.remove("activo"));
    btn.classList.add("activo");
    document.querySelectorAll(".panel-tab").forEach((p) => p.classList.add("oculto"));
    const target = btn.getAttribute("data-tab");
    $(target).classList.remove("oculto");

    if (target === "tab-dashboard") cargarDashboard();
    if (target === "tab-inventario") cargarRecursos();
    if (target === "tab-prestamos") cargarPrestamos();
    if (target === "tab-usuarios") cargarUsuarios();
    if (target === "tab-categorias") cargarCategorias();
    if (target === "tab-sedes") cargarSedes();
    if (target === "tab-historial") cargarHistorial("reciente");
  });
});

// --- 1. INDICADORES ---
async function cargarDashboard() {
  const r = await apiFetch("/api/indicadores");
  if (!r.exito) return;
  const d = r.data;
  const m = d.metricasEstructuras;
  $("badge-undo").textContent = d.operacionesEnPilaUndo;
  $("kpi-contenedor").innerHTML = `
    <div class="kpi-card"><h3>${d.totalRecursos}</h3><small>Total Recursos</small></div>
    <div class="kpi-card teal"><h3>${d.disponibles}</h3><small>Disponibles</small></div>
    <div class="kpi-card accent"><h3>${d.prestados}</h3><small>Prestados</small></div>
    <div class="kpi-card"><h3>${d.mantenimiento}</h3><small>En Mantenimiento</small></div>
    <div class="kpi-card"><h3>${d.totalUsuarios}</h3><small>Usuarios (${d.usuariosSancionados} sanc.)</small></div>
    <div class="kpi-card teal"><h3>${d.prestamosActivos}</h3><small>Préstamos Activos</small></div>
    <div class="kpi-card"><h3>${m.hashFactorCarga.toFixed(3)}</h3><small>Hash α (${m.hashColisiones} col.)</small></div>
    <div class="kpi-card accent"><h3>${m.avlAltura}</h3><small>Altura AVL · Red ${m.grafoRedConexa ? "Conexa" : "Aislada"}</small></div>
  `;
  $("tbody-top").innerHTML = (d.topMasPrestados || []).map((rec) => `
    <tr>
      <td><code>${escapeHtml(rec.codigo)}</code></td>
      <td>${escapeHtml(rec.nombre)}</td>
      <td>${escapeHtml(rec.tipoEspecifico)}</td>
      <td>${escapeHtml(nombreSede(rec.sedeId))}</td>
      <td>${badgeEstado(rec.estado)}</td>
      <td><strong>${rec.vecesPrestado}</strong></td>
    </tr>
  `).join("");
}

// --- 2. INVENTARIO ---
function renderTablaRecursos() {
  const filtro = ($("filtro-rec-vivo").value || "").trim().toLowerCase();
  const filtrados = filtro
    ? cacheRecursos.filter((r) =>
        r.codigo.toLowerCase().includes(filtro) ||
        r.nombre.toLowerCase().includes(filtro) ||
        r.subcategoria.toLowerCase().includes(filtro) ||
        nombreSede(r.sedeId).toLowerCase().includes(filtro) ||
        r.estado.toLowerCase().includes(filtro)
      )
    : cacheRecursos;

  const totalPags = Math.max(1, Math.ceil(filtrados.length / POR_PAGINA));
  if (paginaRec > totalPags) paginaRec = totalPags;
  const inicio = (paginaRec - 1) * POR_PAGINA;
  const slice = filtrados.slice(inicio, inicio + POR_PAGINA);

  $("lbl-total-rec").textContent = `${filtrados.length} registros mostrados`;
  $("lbl-pag-rec").textContent = `Página ${paginaRec} de ${totalPags}`;
  $("btn-prev-rec").disabled = paginaRec <= 1;
  $("btn-next-rec").disabled = paginaRec >= totalPags;

  $("tbody-recursos").innerHTML = slice.map((rec) => `
    <tr>
      <td><code>${escapeHtml(rec.codigo)}</code></td>
      <td>${escapeHtml(rec.nombre)}</td>
      <td>${escapeHtml(rec.tipoEspecifico)}</td>
      <td>${escapeHtml(rec.subcategoria)}</td>
      <td>${escapeHtml(nombreSede(rec.sedeId))}</td>
      <td>${badgeEstado(rec.estado)}</td>
      <td>${rec.vecesPrestado}</td>
      <td>${rec.enEspera}</td>
      <td class="inline-row" style="gap:0.3rem;">
        <button class="btn btn-outline btn-sm" onclick="editarRecursoUI('${escapeHtml(rec.codigo)}')">Editar</button>
        <button class="btn btn-outline btn-sm" onclick="verHistorialIndividual('recurso','${escapeHtml(rec.codigo)}')">Historial</button>
        <button class="btn btn-danger btn-sm" onclick="bajaRecurso('${escapeHtml(rec.codigo)}')">Baja</button>
      </td>
    </tr>
  `).join("") || '<tr><td colspan="9" style="text-align:center;color:var(--muted);">Sin resultados</td></tr>';
}

async function cargarRecursos() {
  const orden = $("sel-orden-rec").value;
  const r = await apiFetch("/api/recursos?orden=" + encodeURIComponent(orden));
  if (!r.exito) return;
  cacheRecursos = r.data || [];
  paginaRec = 1;
  renderTablaRecursos();
}

$("btn-cargar-rec").addEventListener("click", cargarRecursos);
$("filtro-rec-vivo").addEventListener("input", () => { paginaRec = 1; renderTablaRecursos(); });
$("btn-prev-rec").addEventListener("click", () => { if (paginaRec > 1) { paginaRec--; renderTablaRecursos(); } });
$("btn-next-rec").addEventListener("click", () => { paginaRec++; renderTablaRecursos(); });

window.editarRecursoUI = (codigo) => {
  const rec = cacheRecursos.find((x) => x.codigo === codigo);
  if (!rec) return;
  $("det-form-rec").open = true;
  $("rec-cod").value = rec.codigo;
  $("rec-nom").value = rec.nombre;
  $("rec-cat").value = rec.categoria;
  if (rec.estado === "disponible" || rec.estado === "mantenimiento") $("rec-est").value = rec.estado;
  $("rec-sede").value = rec.sedeId;
  $("rec-anio").value = rec.anioAdquisicion;
  $("rec-subcat").value = rec.subcategoria;
  $("rec-detalle").value = rec.detalleExtra;
  $("det-form-rec").scrollIntoView({ behavior: "smooth" });
};

window.bajaRecurso = async (codigo) => {
  if (!confirm(`¿Seguro que deseas dar de baja el recurso ${codigo}?`)) return;
  const r = await apiFetch("/api/recursos/" + encodeURIComponent(codigo), "DELETE");
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { cargarRecursos(); cargarDashboard(); }
};

$("form-buscar-rec").addEventListener("submit", async (e) => {
  e.preventDefault();
  const cod = $("buscar-rec-cod").value.trim();
  const r = await apiFetch("/api/recursos/" + encodeURIComponent(cod));
  const box = $("res-buscar-rec");
  box.classList.remove("oculto");
  if (!r.exito) {
    box.className = "detail-box";
    box.innerHTML = `<span class="badge badge-err">No encontrado</span> ${escapeHtml(r.mensaje)}`;
    return;
  }
  const d = r.data;
  box.className = "detail-box";
  box.innerHTML = `
    <div style="display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:0.5rem;">
      <strong>${escapeHtml(d.codigo)} — ${escapeHtml(d.nombre)}</strong>
      ${badgeEstado(d.estado)}
    </div>
    <div class="detail-grid">
      <div><strong>Tipo:</strong> ${escapeHtml(d.tipoEspecifico)}</div>
      <div><strong>Subcategoría:</strong> ${escapeHtml(d.subcategoria)}</div>
      <div><strong>Sede:</strong> ${escapeHtml(nombreSede(d.sedeId))}</div>
      <div><strong>Año:</strong> ${d.anioAdquisicion}</div>
      <div><strong>Préstamos:</strong> ${d.vecesPrestado}</div>
      <div><strong>En Espera:</strong> ${d.enEspera}</div>
      <div><strong>Detalle:</strong> ${escapeHtml(d.detalleExtra)}</div>
    </div>
    <div style="margin-top:0.6rem;">
      <button class="btn btn-outline btn-sm" onclick="verHistorialIndividual('recurso','${escapeHtml(d.codigo)}')">Ver Historial Individual</button>
    </div>
  `;
});

$("form-reg-rec").addEventListener("submit", async (e) => {
  e.preventDefault();
  const r = await apiFetch("/api/recursos", "POST", {
    codigo: $("rec-cod").value,
    nombre: $("rec-nom").value,
    categoria: $("rec-cat").value,
    estado: $("rec-est").value,
    sedeId: parseInt($("rec-sede").value, 10),
    anioAdquisicion: parseInt($("rec-anio").value, 10),
    subcategoria: $("rec-subcat").value,
    detalleExtra: $("rec-detalle").value
  });
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { cargarRecursos(); cargarDashboard(); }
});

$("btn-actualizar-rec").addEventListener("click", async () => {
  const r = await apiFetch("/api/recursos/" + encodeURIComponent($("rec-cod").value), "PUT", {
    nombre: $("rec-nom").value,
    estado: $("rec-est").value,
    sedeId: parseInt($("rec-sede").value, 10),
    subcategoria: $("rec-subcat").value,
    detalleExtra: $("rec-detalle").value
  });
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { cargarRecursos(); cargarDashboard(); }
});

// --- 3. PRÉSTAMOS Y MATRIZ ---
async function cargarPrestamos() {
  const r = await apiFetch("/api/prestamos");
  if (!r.exito) return;
  $("tbody-prestamos").innerHTML = (r.data || []).map((p) => `
    <tr>
      <td>#${p.idPrestamo}</td>
      <td><code>${escapeHtml(p.codigoRecurso)}</code></td>
      <td><code>${escapeHtml(p.documentoUsuario)}</code></td>
      <td>${escapeHtml(nombreSede(p.sedeId))}</td>
      <td>${escapeHtml(p.fechaPrestamo)}</td>
      <td><button class="btn btn-accent btn-sm" onclick="devolverRapido('${escapeHtml(p.codigoRecurso)}')">Devolver</button></td>
    </tr>
  `).join("") || '<tr><td colspan="6" style="text-align:center;color:var(--muted);">No hay préstamos activos registrados</td></tr>';
}

window.devolverRapido = async (cod) => {
  const r = await apiFetch("/api/devoluciones", "POST", { codigoRecurso: cod });
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { cargarPrestamos(); cargarDashboard(); }
};

$("form-prestamo").addEventListener("submit", async (e) => {
  e.preventDefault();
  const r = await apiFetch("/api/prestamos", "POST", {
    documentoUsuario: $("pre-doc").value,
    codigoRecurso: $("pre-cod").value,
    encolarSiOcupado: $("pre-encolar").checked
  });
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { cargarPrestamos(); cargarDashboard(); }
});

$("form-devolucion").addEventListener("submit", async (e) => {
  e.preventDefault();
  const r = await apiFetch("/api/devoluciones", "POST", { codigoRecurso: $("dev-cod").value });
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { cargarPrestamos(); cargarDashboard(); }
});

$("btn-ver-matriz").addEventListener("click", async () => {
  const cod = $("mat-cod").value.trim();
  const r = await apiFetch("/api/disponibilidad/" + encodeURIComponent(cod));
  if (!r.exito) { mostrarAlerta(r.mensaje, false); return; }
  const dias = ["Lun", "Mar", "Mié", "Jue", "Vie", "Sáb", "Dom"];
  let html = "<table><thead><tr><th>Sede</th>" + dias.map((d) => `<th>${d}</th>`).join("") + "</tr></thead><tbody>";
  for (const s of r.data.sedes) {
    html += `<tr><td><strong>${escapeHtml(s.nombre)}</strong></td>`;
    s.dias.forEach((libre, idxDia) => {
      html += libre
        ? `<td><button class="btn btn-outline btn-sm" onclick="reservarCelda('${escapeHtml(cod)}', ${s.sedeId}, ${idxDia})">Libre</button></td>`
        : `<td><span class="badge badge-err">Ocup.</span></td>`;
    });
    html += "</tr>";
  }
  html += "</tbody></table>";
  $("contenedor-matriz").innerHTML = html;
});

window.reservarCelda = async (cod, sedeId, dia) => {
  const r = await apiFetch("/api/disponibilidad/reservar", "POST", { codigoRecurso: cod, sedeId, dia });
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { $("btn-ver-matriz").click(); cargarDashboard(); }
};

// --- 4. USUARIOS (Incluye Buscador O(1) RQF 010) ---
$("usu-es-admin").addEventListener("change", () => {
  $("usu-clave").classList.toggle("oculto", !$("usu-es-admin").checked);
});

$("form-buscar-usu").addEventListener("submit", async (e) => {
  e.preventDefault();
  const doc = $("buscar-usu-doc").value.trim();
  const r = await apiFetch("/api/usuarios/" + encodeURIComponent(doc));
  const box = $("res-buscar-usu");
  box.classList.remove("oculto");
  box.className = "detail-box";
  if (!r.exito) {
    box.innerHTML = `<span class="badge badge-err">No encontrado</span> ${escapeHtml(r.mensaje)}`;
    return;
  }
  const u = r.data;
  box.innerHTML = `
    <div style="display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:0.5rem;">
      <strong>${escapeHtml(u.documento)} — ${escapeHtml(u.nombre)} ${escapeHtml(u.apellido)}</strong>
      <span>${u.esAdmin ? '<span class="badge badge-info">Administrador</span>' : '<span class="badge badge-ok">Usuario CRAI</span>'}
            ${u.sancionado ? '<span class="badge badge-err">Sancionado</span>' : ''}</span>
    </div>
    <div style="margin-top:0.6rem;">
      <button class="btn btn-outline btn-sm" onclick="verHistorialIndividual('usuario','${escapeHtml(u.documento)}')">Ver Historial Individual</button>
    </div>
  `;
});

async function cargarUsuarios() {
  const r = await apiFetch("/api/usuarios");
  if (!r.exito) return;
  $("tbody-usuarios").innerHTML = (r.data || []).map((u) => `
    <tr>
      <td><code>${escapeHtml(u.documento)}</code></td>
      <td>${escapeHtml(u.nombre)} ${escapeHtml(u.apellido)}</td>
      <td>${u.esAdmin ? '<span class="badge badge-info">Administrador</span>' : 'Usuario'}</td>
      <td>${u.sancionado ? '<span class="badge badge-err">Sancionado</span>' : '<span class="badge badge-ok">Habilitado</span>'}</td>
      <td class="inline-row" style="gap:0.3rem;">
        <button class="btn btn-outline btn-sm" onclick="precargarUsuario('${escapeHtml(u.documento)}','${escapeHtml(u.nombre)}','${escapeHtml(u.apellido)}',${u.sancionado})">Editar</button>
        <button class="btn btn-outline btn-sm" onclick="verHistorialIndividual('usuario','${escapeHtml(u.documento)}')">Historial</button>
        ${u.esAdmin ? "" : `<button class="btn btn-danger btn-sm" onclick="bajaUsuario('${escapeHtml(u.documento)}')">Baja</button>`}
      </td>
    </tr>
  `).join("");
}

window.precargarUsuario = (doc, nom, ape, sanc) => {
  $("upd-usu-doc").value = doc;
  $("upd-usu-nom").value = nom;
  $("upd-usu-ape").value = ape;
  $("upd-usu-sanc").checked = !!sanc;
};

window.bajaUsuario = async (doc) => {
  if (!confirm(`¿Seguro que deseas dar de baja al usuario ${doc}?`)) return;
  const r = await apiFetch("/api/usuarios/" + encodeURIComponent(doc), "DELETE");
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { cargarUsuarios(); cargarDashboard(); }
};

$("form-reg-usu").addEventListener("submit", async (e) => {
  e.preventDefault();
  const r = await apiFetch("/api/usuarios", "POST", {
    documento: $("usu-doc").value,
    nombre: $("usu-nom").value,
    apellido: $("usu-ape").value,
    esAdmin: $("usu-es-admin").checked,
    clave: $("usu-clave").value
  });
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { cargarUsuarios(); cargarDashboard(); }
});

$("form-upd-usu").addEventListener("submit", async (e) => {
  e.preventDefault();
  const r = await apiFetch("/api/usuarios/" + encodeURIComponent($("upd-usu-doc").value), "PUT", {
    nombre: $("upd-usu-nom").value,
    apellido: $("upd-usu-ape").value,
    sancionado: $("upd-usu-sanc").checked
  });
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) { cargarUsuarios(); cargarDashboard(); }
});

// --- 5. CATEGORÍAS (Árbol visual + tabla, cero JSON crudo) ---
async function cargarCategorias() {
  const r = await apiFetch("/api/categorias");
  if (!r.exito) return;
  $("lista-jerarquia").innerHTML = (r.data || []).map((linea) => {
    const espacios = linea.length - linea.trimStart().length;
    const nivel = Math.floor(espacios / 2);
    const nombre = linea.trim();
    return `<li class="tree-item" style="padding-left:${0.6 + nivel * 1.15}rem;" onclick="consultarCategoriaUI('${escapeHtml(nombre)}')">
      ${nivel > 0 ? "↳ " : "📁 "}<strong>${escapeHtml(nombre)}</strong>
    </li>`;
  }).join("");
}

window.consultarCategoriaUI = async (nombre) => {
  $("cat-consulta").value = nombre;
  const r = await apiFetch("/api/categorias/" + encodeURIComponent(nombre) + "/recursos");
  const cont = $("res-cat");
  if (!r.exito) {
    cont.innerHTML = `<div class="detail-box" style="border-left-color:#991b1b;"><span class="badge badge-err">404</span> ${escapeHtml(r.mensaje)}</div>`;
    return;
  }
  const recs = r.data.recursos || [];
  cont.innerHTML = `
    <p style="margin:0 0 0.5rem;">Categoría <strong>${escapeHtml(r.data.categoria)}</strong>: <strong>${r.data.total}</strong> recursos encontrados.</p>
    <div class="tabla-wrap">
      <table>
        <thead><tr><th>Código</th><th>Nombre</th><th>Subcategoría</th><th>Sede</th><th>Estado</th></tr></thead>
        <tbody>
          ${recs.slice(0, 40).map((x) => `
            <tr>
              <td><code>${escapeHtml(x.codigo)}</code></td>
              <td>${escapeHtml(x.nombre)}</td>
              <td>${escapeHtml(x.subcategoria)}</td>
              <td>${escapeHtml(nombreSede(x.sedeId))}</td>
              <td>${badgeEstado(x.estado)}</td>
            </tr>
          `).join("") || '<tr><td colspan="5" style="text-align:center;color:var(--muted);">Sin recursos asociados aún</td></tr>'}
        </tbody>
      </table>
    </div>
  `;
};

$("form-cat").addEventListener("submit", async (e) => {
  e.preventDefault();
  const r = await apiFetch("/api/categorias", "POST", {
    padre: $("cat-padre").value,
    hija: $("cat-hija").value
  });
  mostrarAlerta(r.mensaje, r.exito);
  if (r.exito) cargarCategorias();
});

$("form-buscar-cat").addEventListener("submit", (e) => {
  e.preventDefault();
  consultarCategoriaUI($("cat-consulta").value.trim());
});

// --- 6. SEDES Y BFS (Tarjetas y pasos visuales, cero JSON crudo) ---
async function cargarSedes() {
  const r = await apiFetch("/api/sedes");
  if (!r.exito) return;
  $("lbl-conexa").textContent = r.data.redConexa ? "DFS: Red 100% Conexa" : "DFS: Hay sedes aisladas";
  $("lista-sedes").innerHTML = (r.data.sedes || []).map((s) => `
    <div class="detail-box" style="margin-top:0;">
      <strong>[${s.id}] ${escapeHtml(s.nombre)}</strong><br>
      <small style="color:var(--muted);">${escapeHtml(s.direccion)}</small>
      <div style="margin-top:0.45rem; font-size:0.79rem;">
        ${s.conexiones.map((c) => `<span class="badge badge-info" style="margin:0.15rem 0.2rem 0 0;">↔ ${escapeHtml(c.destinoNombre)} (${(c.distanciaMetros/1000).toFixed(1)} km)</span>`).join("")}
      </div>
    </div>
  `).join("");
}

$("form-bfs-rec").addEventListener("submit", async (e) => {
  e.preventDefault();
  const q = `origen=${encodeURIComponent($("bfs-rec-origen").value)}&codigo=${encodeURIComponent($("bfs-rec-cod").value)}&maxDist=${encodeURIComponent($("bfs-rec-max").value)}`;
  const r = await apiFetch("/api/sedes/buscar-recurso?" + q);
  const box = $("res-bfs-rec");
  box.classList.remove("oculto");
  box.className = "detail-box";
  if (!r.exito || !r.data.encontrada) {
    box.innerHTML = `<span class="badge badge-err">Sin disponibilidad</span> ${escapeHtml(r.mensaje)}`;
    return;
  }
  const d = r.data;
  box.innerHTML = `
    <div style="display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:0.4rem;">
      <strong>Disponible en: ${escapeHtml(d.nombreSedeDestino)}</strong>
      ${d.esViable
        ? '<span class="badge badge-ok">Traslado Viable</span>'
        : '<span class="badge badge-warn">Fuera de rango rápido</span>'}
    </div>
    <small style="color:var(--muted);">Saltos: ${d.saltos} · Distancia acumulada: ${d.distanciaTotalMetros} m (${(d.distanciaTotalMetros/1000).toFixed(1)} km)</small>
    <div class="route-steps">
      ${(d.ruta || []).map((n) => `<span class="route-node">${escapeHtml(n)}</span>`).join("<span>→</span>")}
    </div>
  `;
});

$("form-bfs-ruta").addEventListener("submit", async (e) => {
  e.preventDefault();
  const q = `origen=${encodeURIComponent($("bfs-orig").value)}&destino=${encodeURIComponent($("bfs-dest").value)}`;
  const r = await apiFetch("/api/sedes/ruta?" + q);
  const box = $("res-bfs-ruta");
  box.classList.remove("oculto");
  box.className = "detail-box";
  if (!r.exito) {
    box.innerHTML = `<span class="badge badge-err">Error</span> ${escapeHtml(r.mensaje)}`;
    return;
  }
  box.innerHTML = `
    <strong>Camino más corto (${r.data.saltos} salto${r.data.saltos === 1 ? "" : "s"})</strong>
    <div class="route-steps">
      ${(r.data.ruta || []).map((n) => `<span class="route-node">${escapeHtml(n)}</span>`).join("<span>→</span>")}
    </div>
  `;
});

// --- 7. HISTORIAL GENERAL E INDIVIDUAL ---
async function cargarHistorial(orden) {
  const r = await apiFetch("/api/historial?orden=" + encodeURIComponent(orden));
  if (!r.exito) return;
  $("tbody-historial").innerHTML = (r.data || []).map((h) => `
    <tr>
      <td>${escapeHtml(h.fechaHora)}</td>
      <td><span class="badge badge-info">${escapeHtml(h.tipoEvento)}</span></td>
      <td><code>${escapeHtml(h.codigoRecurso)}</code></td>
      <td><code>${escapeHtml(h.documentoUsuario)}</code></td>
      <td>${escapeHtml(h.detalle)}</td>
    </tr>
  `).join("") || '<tr><td colspan="5" style="text-align:center;color:var(--muted);">Sin eventos registrados</td></tr>';
}

$("btn-hist-reciente").addEventListener("click", () => cargarHistorial("reciente"));
$("btn-hist-antiguo").addEventListener("click", () => cargarHistorial("antiguo"));

window.verHistorialIndividual = async (tipo, id) => {
  const ruta = tipo === "recurso"
    ? "/api/historial/recurso/" + encodeURIComponent(id)
    : "/api/historial/usuario/" + encodeURIComponent(id);
  const r = await apiFetch(ruta);
  $("modal-hist-titulo").textContent = `Historial Individual (${tipo === "recurso" ? "Recurso" : "Usuario"}: ${id})`;
  if (!r.exito || !(r.data || []).length) {
    $("modal-hist-body").innerHTML = `<p style="color:var(--muted);">${escapeHtml(r.exito ? "Aún no tiene movimientos registrados." : r.mensaje)}</p>`;
  } else {
    $("modal-hist-body").innerHTML = `
      <div class="tabla-wrap">
        <table>
          <thead><tr><th>Fecha</th><th>Evento</th><th>Detalle</th></tr></thead>
          <tbody>
            ${r.data.map((h) => `
              <tr>
                <td>${escapeHtml(h.fechaHora)}</td>
                <td><span class="badge badge-info">${escapeHtml(h.tipoEvento)}</span></td>
                <td>${escapeHtml(h.detalle)}</td>
              </tr>
            `).join("")}
          </tbody>
        </table>
      </div>
    `;
  }
  $("modal-hist").classList.remove("oculto");
};

$("btn-cerrar-modal").addEventListener("click", () => $("modal-hist").classList.add("oculto"));
$("modal-hist").addEventListener("click", (e) => { if (e.target === $("modal-hist")) $("modal-hist").classList.add("oculto"); });
