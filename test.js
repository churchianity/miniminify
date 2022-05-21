
// ws connection
let ws = null;

// networked/synced data
let cohorts = []; // doesn't change often.

// changes often.
let queueItems = [];
let instructorItems = [];

// client isolated data
const intervalIds = {};
let user = null;

// render helpers
function renderAsideContent() {
    const aside = document.querySelector("aside");
    aside.replaceChildren();

    if (user) {
        const asideName = document.createElement("div")
        asideName.className = "aside-name";
        asideName.innerText = user.name;
        aside.appendChild(asideName);

        const asideCohort = document.createElement("div")
        asideCohort.className = "aside-cohort";
        asideCohort.innerText = user.cohort;
        aside.appendChild(asideCohort);

        if (user.instructor) {
            const brbButton = document.createElement("button")
            brbButton.className = "aside-button";
            brbButton.innerHTML = `
                <svg stroke="currentColor" fill="currentColor" stroke-width="0" viewBox="0 0 640 512" height="1em" width="1em" xmlns="http://www.w3.org/2000/svg"><path d="M192 384h192c53 0 96-43 96-96h32c70.6 0 128-57.4 128-128S582.6 32 512 32H120c-13.3 0-24 10.7-24 24v232c0 53 43 96 96 96zM512 96c35.3 0 64 28.7 64 64s-28.7 64-64 64h-32V96h32zm47.7 384H48.3c-47.6 0-61-64-36-64h583.3c25 0 11.8 64-35.9 64z"></path></svg>
                <span class="aside-button-text">BRB</span>
            `;
            aside.appendChild(brbButton)

            const availableButton = document.createElement("button")
            availableButton.className = "aside-button";
            function setAvailable(_available) {
                user.available = _available;

                if (user.available) {
                    availableButton.innerHTML = `
                        <svg stroke="currentColor" fill="currentColor" stroke-width="0" viewBox="0 0 576 512" height="1em" width="1em" xmlns="http://www.w3.org/2000/svg"><path d="M384 64H192C86 64 0 150 0 256s86 192 192 192h192c106 0 192-86 192-192S490 64 384 64zm0 320c-70.8 0-128-57.3-128-128 0-70.8 57.3-128 128-128 70.8 0 128 57.3 128 128 0 70.8-57.3 128-128 128z"></path></svg>
                        <span class="aside-button-text">Available</span>
                    `;
                    if (user.instructor) {
                        instructorItems.push(user);
                    }
                } else {
                    availableButton.innerHTML = `
                        <svg stroke="currentColor" fill="currentColor" stroke-width="0" viewBox="0 0 576 512" height="1em" width="1em" xmlns="http://www.w3.org/2000/svg"><path d="M384 64H192C85.961 64 0 149.961 0 256s85.961 192 192 192h192c106.039 0 192-85.961 192-192S490.039 64 384 64zM64 256c0-70.741 57.249-128 128-128 70.741 0 128 57.249 128 128 0 70.741-57.249 128-128 128-70.741 0-128-57.249-128-128zm320 128h-48.905c65.217-72.858 65.236-183.12 0-256H384c70.741 0 128 57.249 128 128 0 70.74-57.249 128-128 128z"></path></svg>
                        <span class="aside-button-text">Not Available</span>
                    `;
                    if (user.instructor) {
                        const index = instructorItems.findIndex(i => i.id === user.id);

                        if (index !== -1) {
                            instructorItems.splice(index, 1);
                        }
                    }
                }

                updateInstructors();
                renderMainContent();
            }
            setAvailable(user.available);
            availableButton.onclick = () => setAvailable(!user.available);
            aside.appendChild(availableButton);
        }
    }
}

function renderHeaderMenu() {
    const header = document.querySelector(".header-menu");

    if (!user) {
        const signIn = document.createElement("span")
        signIn.className = "menu-item";
        signIn.innerText = "Sign In";
        signIn.onclick = () => {
            const accessKey = prompt("Enter your access key: ");
            sendWsJson(ws, { type: "login", data: accessKey });
        }

        header.replaceChildren(signIn);

    } else {
        const profile = document.createElement("span")
        profile.className = "menu-item";
        profile.innerText = "Profile";
        profile.onclick = () => {
            const profileEditDropdown = document.createElement("div")
            profileEditDropdown.className = "profile-edit-dropdown";
            profileEditDropdown.onclick = (e) => {
                e.stopImmediatePropagation();
            }

            const profileEditDropdownHeading = document.createElement("label");
            profileEditDropdownHeading.className = "profile-edit-heading";
            profileEditDropdownHeading.innerText = "Edit Profile";
            profileEditDropdown.appendChild(profileEditDropdownHeading);

            const nameInput = document.createElement("input");
            nameInput.className = "profile-edit-input";
            nameInput.type = "text";
            nameInput.value = user.name;
            profileEditDropdown.appendChild(nameInput);

            const cohortInput = document.createElement("select");
            cohortInput.className = "profile-edit-input";
            cohortInput.value = user.cohort;
            for (let i = 0; i < cohorts.length; i++) {
                const cohort = document.createElement("option");
                cohort.value = cohorts[i];
                cohort.innerText = cohorts[i];
                cohortInput.appendChild(cohort);
            }
            profileEditDropdown.appendChild(cohortInput);

            const closeButton = document.createElement("button");
            closeButton.className = "profile-edit-closeButton";
            closeButton.innerText = "Close & Save";
            closeButton.onclick = (e) => {
                e.stopImmediatePropagation();

                user.name = nameInput.value;
                user.cohort = cohortInput.value;
                renderUserDependentContent();

                profile.replaceChildren("Profile");

            }
            profileEditDropdown.appendChild(closeButton);

            profile.appendChild(profileEditDropdown);
        }

        const logout = document.createElement("span")
        logout.className = "menu-item";
        logout.innerText = "Sign Out";
        logout.onclick = () => {
            if (user.instructor) {
                user.available = false;
                updateInstructors();
            }

            user = null;
            renderUserDependentContent();
        }

        const circle = document.createElement("div")
        circle.className = "circle";

        header.replaceChildren(profile, logout, circle);
    }
}

function renderQueue() {
    const container = document.querySelector(".queue-container");
    container.replaceChildren();

    if (queueItems.length === 0) {
        container.innerHTML = `<li class="empty-item">The Queue is Empty!</li>`

    } else {
        for (let i = 0; i < queueItems.length; i++) {
            const data = queueItems[i];

            const hours = ("" + data.joinedAt.getHours()).padStart(2, "0");
            const minutes = ("" + data.joinedAt.getMinutes()).padStart(2, "0");

            const queueItem = document.createElement("li");
            queueItem.className = "main-content-item";
            queueItem.innerHTML = `
                <div class="main-content-item-topRow">
                  <div class="main-content-item-topRowLeft">
                    <label class="main-content-item-name">${data.name}</label>
                    <label class="main-content-item-query">${data.query}</label>
                  </div>
                  <label class="main-content-item-timestamp">${hours}:${minutes}</label>
                </div>
            `;

            if (user.instructor && user.available && !user.call) {
                const actionRow = document.createElement("div");
                actionRow.className = "main-content-item-actionRow";

                const actionButton = document.createElement("button");
                actionButton.className = "main-content-item-actionButton";
                actionButton.innerText = "call";
                actionButton.onclick = () => {
                    queueItems.splice(queueItems.findIndex(q => q.name === data.name), 1);
                    data.calledAt = new Date();
                    user.call = data;

                    updateQueue();
                    updateInstructors();
                    renderMainContent();
                }
                actionRow.appendChild(actionButton);

                queueItem.appendChild(actionRow);
            }

            container.appendChild(queueItem);
        }
        if (!user.instructor) {
            const joinQueueButton = document.createElement("button");

            joinQueueButton.className = "main-content-item-actionButton";
            joinQueueButton.innerText = "Join Queue";
            joinQueueButton.onclick = () => {
                renderMainContent();
            }
            container.appendChild(joinQueueButton);
        }
    }
}

function renderInstructors() {
    const container = document.querySelector(".instructors-container");
    container.replaceChildren();

    if (instructorItems.length === 0) {
        container.innerHTML = `<li class="empty-item">There are no instructors online at the moment.</li>`;

    } else {
        const availableInstructors = instructorItems.filter(i => !!i.available);
        for (let i = 0; i < availableInstructors.length; i++) {
            const data = availableInstructors[i];

            const queueItem = document.createElement("li");
            queueItem.className = "main-content-item";

            const topRow = document.createElement("div");
            topRow.className = "main-content-item-topRow";
            topRow.innerHTML = `
                <div class="main-content-item-topRowLeft">
                  <label class="main-content-item-name">${data.name}</label>
                  <label class="main-content-item-query">${data.call ? data.call.name : 'is Available!'}</label>
                </div>
            `;

            const timestamp = document.createElement("label");
            timestamp.className = "main-content-item-timestamp";
            if (data.call) {
                intervalIds[data.call.name] = setInterval(() => {
                    const diff = new Date(new Date() - data.call.calledAt);
                    const minutes = ("" + diff.getMinutes()).padStart(2, "0");
                    const seconds = ("" + diff.getSeconds()).padStart(2, "0");
                    timestamp.innerText = `${minutes}:${seconds}`;
                }, 1000);
            }
            topRow.appendChild(timestamp);

            const actionRow = document.createElement("div");
            actionRow.className = "main-content-item-actionRow";

            if (data.call) {
                const actionButton = document.createElement("button");
                actionButton.className = "main-content-item-actionButton";
                actionButton.innerText = "resolved";
                actionButton.onclick = () => {
                    clearInterval(intervalIds[data.call.name])
                    user.call = null;
                    updateInstructors();
                    renderMainContent();
                }
                actionRow.appendChild(actionButton);
            }

            queueItem.appendChild(topRow);
            queueItem.appendChild(actionRow);

            container.appendChild(queueItem);
        }
    }
}

function renderMainContent() {
    const queue = document.querySelector(".queue");
    queue.style.display = "none";
    const instructors = document.querySelector(".instructors");
    instructors.style.display = "none";

    if (user) {
        renderQueue();
        queue.style.display = "block";

        if (user.instructor) {
            renderInstructors();
            instructors.style.display = "block";
        }
    }
}

function renderUserDependentContent() {
    renderHeaderMenu();
    renderAsideContent();
    renderMainContent();
}

////////////////////////////////////////////////////////////////////////////////
// websocket stuff below here
function sendWsJson(ws, object) {
    if (ws) {
        ws.send(JSON.stringify(object));
    }
}

function updateInstructors() {
    sendWsJson(ws, { type: "instructorItems", data: instructorItems });
}

function updateQueue() {
    sendWsJson(ws, { type: "queueItems", data: queueItems });
}

function handleWsMessage(event) {
    const message = JSON.parse(event.data);

    switch (message.type) {
        case "cohorts": {
            cohorts = message.data;
        } break;
        case "login": {
            user = message.data;
            renderUserDependentContent();
        } break;
        case "instructorItems": {
            instructorItems = message.data.filter(i => {
                if (i.cohort === user.cohort) {
                    if (i.call) {
                        console.log(i);
                        i.call.calledAt = new Date(i.call.calledAt);
                        i.call.joinedAt = new Date(i.call.joinedAt);
                    }
                    return true;
                }
            });

            if (user.instructor) {
                // something about ourselves, the 'user' could have changed, re-calc who we are
                // if we don't find ourselves in the list of instructors, we probably aren't available, in which case it's fine, and we don't need to do anything.
                const index = instructorItems.findIndex(i => i.id === user.id);
                if (index !== -1) {
                    user = instructorItems[index];
                }
            }
            renderMainContent();
        } break;
        case "queueItems": {
            queueItems = message.data.filter(item => {
                // discard queue items if we aren't in the correct cohort (or aren't logged in)
                if (user && item.cohort === user.cohort) {
                    item.joinedAt = new Date(item.joinedAt);

                    if (item.calledAt) {
                        item.calledAt = new Date(item.calledAt);
                    }

                    return true;
                }
            });
            renderMainContent();
        } break;
    }
}

function init() {
    // set up websocket stuff
    ws = new WebSocket("ws://localhost:42069");

    ws.addEventListener("open", event => {
        ws.addEventListener("message", handleWsMessage);
    });

    renderUserDependentContent();
}

window.onload = init;
